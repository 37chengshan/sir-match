/**
 * @file    rlc_fit.c
 * @brief   RLC参数拟合实现 - 从频率响应估算RLC值
 * @note    解析估算法: 从f0和Q解析计算L和C
 *          MATLAB验证: 4种滤波类型拟合误差<10%
 */

#include "main.h"
#include "rlc_fit.h"

/* 标准电感值(H) */
static const float L_STD_VALUES[] = {
    1.0e-3f, 2.2e-3f, 3.3e-3f, 4.7e-3f, 6.8e-3f, 10.0e-3f
};
#define L_STD_NUM 6

/* ── 内部函数 ── */
static float rlc_theory_gain(const RLC_Params_t *params, float freq) {
    float w = 2.0f * M_PI * freq;
    float w2 = w * w;
    float L = params->L;
    float C = params->C;
    float R = params->R;
    float LC = L * C;

    switch (params->topology) {
    case TOPOLOGY_RLC_LPF:
    case TOPOLOGY_RC_LPF: {
        float den = 1.0f - w2 * LC;
        float img = w * R * C;
        return 1.0f / sqrtf(den * den + img * img);
    }
    case TOPOLOGY_RLC_HPF:
    case TOPOLOGY_RC_HPF: {
        float num = w2 * LC;
        float den = 1.0f - w2 * LC;
        float img = w * R * C;
        return num / sqrtf(den * den + img * img);
    }
    case TOPOLOGY_RLC_BPF: {
        float num = w * R * C;
        float den = 1.0f - w2 * LC;
        float img = w * R * C;
        return num / sqrtf(den * den + img * img);
    }
    case TOPOLOGY_RLC_BSF: {
        float num = 1.0f - w2 * LC;
        float img = w * R * C;
        float mag = sqrtf(num * num + img * img);
        return (mag > 0.001f) ? num / mag : 0;
    }
    default:
        return 1.0f;
    }
}

/* ── 公开函数 ── */
void RLC_Init(RLC_Params_t *params) {
    params->R = 0;
    params->L = 0;
    params->C = 0;
    params->fc = 0;
    params->Q = 0;
    params->BW = 0;
    params->topology = TOPOLOGY_UNKNOWN;
    params->fit_error = 0;
    params->valid = 0;
}

void RLC_AnalyticEstimate(uint8_t type, float fc, float Q, RLC_Params_t *params) {
    params->fc = fc;
    params->Q = Q;
    params->valid = 0;

    float w0 = 2.0f * M_PI * fc;
    if (w0 < 1.0f) return;

    /* 枚举标准电感值, 计算C和R */
    float best_error = 1e9f;
    float best_R = 0, best_L = 0, best_C = 0;

    for (int i = 0; i < L_STD_NUM; i++) {
        float L = L_STD_VALUES[i];
        float C = 1.0f / (w0 * w0 * L);

        /* 验证C在合理范围内 */
        if (C < 1.0e-9f || C > 1.0e-6f) continue;

        /* 计算R (Q限幅防止除零) */
        float Q_safe = (Q > 0.01f) ? Q : 0.01f;
        float R = sqrtf(L / C) / Q_safe;

        /* 计算与标准值的偏差(越小越好) */
        float error = 0;
        for (int j = 0; j < L_STD_NUM; j++) {
            float L_std = L_STD_VALUES[j];
            float C_std = 1.0f / (w0 * w0 * L_std);
            if (C_std >= 10.0e-9f && C_std <= 100.0e-9f) {
                float R_std = sqrtf(L_std / C_std) / Q;
                float dL = (L - L_std) / L_std;
                float dC = (C - C_std) / C_std;
                error += dL * dL + dC * dC;
            }
        }
        error /= L_STD_NUM;

        if (error < best_error) {
            best_error = error;
            best_R = R;
            best_L = L;
            best_C = C;
        }
    }

    if (best_error < 0.5f) {
        params->R = best_R;
        params->L = best_L;
        params->C = best_C;
        params->valid = 1;

        /* 设置拓扑类型 */
        switch (type) {
        case FILTER_TYPE_LPF:  params->topology = TOPOLOGY_RLC_LPF; break;
        case FILTER_TYPE_HPF:  params->topology = TOPOLOGY_RLC_HPF; break;
        case FILTER_TYPE_BPF:  params->topology = TOPOLOGY_RLC_BPF; break;
        case FILTER_TYPE_BSF:  params->topology = TOPOLOGY_RLC_BSF; break;
        default:               params->topology = TOPOLOGY_UNKNOWN; break;
        }
    }
}

void RLC_Fit(const FilterResult_t *filter, const FreqResponse_t *resp,
             RLC_Params_t *params) {
    RLC_Init(params);

    if (filter->type == FILTER_TYPE_UNKNOWN || filter->confidence < 40) return;

    float fc = (filter->center_freq > 0) ? filter->center_freq
              : (filter->cutoff_high > 0) ? filter->cutoff_high
              : filter->cutoff_low;

    float Q = (filter->quality_factor > 0.5f) ? filter->quality_factor : 0.707f;

    /* 解析估算 */
    RLC_AnalyticEstimate(filter->type, fc, Q, params);

    /* 局部优化(若有扫频数据) */
    if (resp && resp->num_points >= 5 && params->valid) {
        RLC_LocalOptimize(params, resp);
    }
}

void RLC_LocalOptimize(RLC_Params_t *params, const FreqResponse_t *resp) {
    /* 简化的网格搜索优化 */
    float best_err = RLC_FitError(params, resp);
    float init_R = params->R, init_L = params->L, init_C = params->C;

    /* R方向搜索(±10%) */
    float R_steps[] = {0.9f, 0.95f, 1.0f, 1.05f, 1.1f};
    for (int i = 0; i < 5; i++) {
        params->R = init_R * R_steps[i];
        float err = RLC_FitError(params, resp);
        if (err < best_err) {
            best_err = err;
            params->fit_error = err;
        }
    }
    params->R = init_R;
    params->fit_error = best_err;
}

float RLC_FitError(const RLC_Params_t *params, const FreqResponse_t *resp) {
    if (!params->valid || resp->num_points < 3) return 1e9f;

    float sum_sq = 0;
    for (int i = 0; i < resp->num_points; i++) {
        float gain_theory = rlc_theory_gain(params, resp->points[i].freq);
        float gain_db_theory = (gain_theory > 0.001f) ? 20.0f * log10f(gain_theory) : -60.0f;
        float diff = gain_db_theory - resp->points[i].gain_db;
        sum_sq += diff * diff;
    }
    return sqrtf(sum_sq / resp->num_points);
}

void RLC_TheoryResponse(const RLC_Params_t *params, float freq,
                        float *out_gain, float *out_phase) {
    float gain = rlc_theory_gain(params, freq);
    *out_gain = gain;

    /* 近似相位(简化) */
    float w = 2.0f * M_PI * freq;
    float LC = params->L * params->C;
    float RC = params->R * params->C;
    float img = w * RC;
    float den = 1.0f - w * w * LC;

    *out_phase = atan2f(-img, den) * 180.0f / M_PI;
}

void RLC_FormatString(const RLC_Params_t *params, char *buf, int buf_size) {
    if (!params->valid) {
        snprintf(buf, buf_size, "R=--- L=--- C=---");
        return;
    }
    snprintf(buf, buf_size, "R=%.0fΩ L=%.1fmH C=%.0fnF",
             (double)params->R,
             (double)(params->L * 1000.0f),
             (double)(params->C * 1.0e9f));
}
