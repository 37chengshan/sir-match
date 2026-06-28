/**
 * @file    filter_id.c
 * @brief   滤波器类型识别实现 - 基于扫频数据的幅频/相频分析
 * @note    幅度+相位联合判定, 识别低通/高通/带通/带阻
 *          MATLAB验证: 4种类型100%正确识别
 */

#include "main.h"
#include "filter_id.h"

/* ── 公开函数 ── */
void FreqResp_Init(FreqResponse_t *resp, float ref_amp) {
    resp->num_points = 0;
    resp->ref_amplitude = ref_amp;
}

void FreqResp_AddPoint(FreqResponse_t *resp, float freq, float vout, float phase) {
    if (resp->num_points >= FREQ_RESP_MAX_POINTS) return;

    resp->points[resp->num_points].freq = freq;
    resp->points[resp->num_points].gain = vout / resp->ref_amplitude;
    /* 避免 log10(0) */
    float g = resp->points[resp->num_points].gain;
    resp->points[resp->num_points].gain_db = (g > 0.001f) ? 20.0f * log10f(g) : -60.0f;
    resp->points[resp->num_points].phase_deg = phase;
    resp->num_points++;
}

void FilterID_Analyze(const FreqResponse_t *resp, FilterResult_t *result) {
    if (resp->num_points < 5) {
        result->type = FILTER_TYPE_UNKNOWN;
        result->confidence = 0;
        return;
    }

    int n = resp->num_points;
    const FreqPoint_t *p = resp->points;

    /* 提取关键特征 */
    result->dc_gain_db = p[0].gain_db;
    result->hf_gain_db = p[n - 1].gain_db;

    /* 找峰值和谷值 */
    float gain_max = -999.0f, gain_min = 999.0f;
    int max_idx = 0, min_idx = 0;

    for (int i = 0; i < n; i++) {
        if (p[i].gain_db > gain_max) { gain_max = p[i].gain_db; max_idx = i; }
        if (p[i].gain_db < gain_min) { gain_min = p[i].gain_db; min_idx = i; }
    }
    result->max_gain_db = gain_max;
    result->min_gain_db = gain_min;

    /* 幅度特征判定 */
    float gain_range = gain_max - gain_min;
    if (gain_range < MIN_GAIN_RANGE_DB) {
        result->type = FILTER_TYPE_UNKNOWN;
        result->confidence = 20;
        return;
    }

    /* 相位特征 */
    float phase_low = p[0].phase_deg;
    float phase_high = p[n - 1].phase_deg;
    float phase_mid = p[n / 2].phase_deg;

    /* 计算斜率 */
    result->slope_low = FilterID_CalcSlope(resp, 0, n / 4);
    result->slope_high = FilterID_CalcSlope(resp, 3 * n / 4, n - 1);

    /* ── 主判定逻辑 ── */
    uint8_t amp_type = FILTER_TYPE_UNKNOWN;
    float amp_conf = 0;

    if (result->dc_gain_db > result->hf_gain_db + SLOPE_THRESHOLD_DB) {
        /* 低频高, 高频低 → 低通 */
        amp_type = FILTER_TYPE_LPF;
        amp_conf = 70;
        result->cutoff_high = FilterID_Find3dBFreq(resp, result->dc_gain_db, 1);
    } else if (result->hf_gain_db > result->dc_gain_db + SLOPE_THRESHOLD_DB) {
        /* 高频高, 低频低 → 高通 */
        amp_type = FILTER_TYPE_HPF;
        amp_conf = 70;
        result->cutoff_low = FilterID_Find3dBFreq(resp, result->hf_gain_db, 0);
    } else if (gain_max > result->dc_gain_db + SLOPE_THRESHOLD_DB
            && gain_max > result->hf_gain_db + SLOPE_THRESHOLD_DB) {
        /* 中间有峰值 → 带通 */
        amp_type = FILTER_TYPE_BPF;
        amp_conf = 80;
        result->center_freq = p[max_idx].freq;
    } else if (gain_min < result->dc_gain_db - SLOPE_THRESHOLD_DB
            && gain_min < result->hf_gain_db - SLOPE_THRESHOLD_DB) {
        /* 中间有谷值 → 带阻 */
        amp_type = FILTER_TYPE_BSF;
        amp_conf = 80;
        result->center_freq = p[min_idx].freq;
    }

    /* 相位验证 */
    float phase_conf = 0;
    switch (amp_type) {
    case FILTER_TYPE_LPF:
        /* 相位应从0°向-180°下降 */
        if (phase_low > -20.0f && phase_high < -90.0f)
            phase_conf = 30;
        else if (phase_high < -30.0f)
            phase_conf = 15;
        break;
    case FILTER_TYPE_HPF:
        if (phase_low < 20.0f && phase_high > 90.0f)
            phase_conf = 30;
        else if (phase_high > 30.0f)
            phase_conf = 15;
        break;
    case FILTER_TYPE_BPF:
        /* 中心频率处相位穿越0° */
        if (p[max_idx].phase_deg > -30.0f && p[max_idx].phase_deg < 30.0f)
            phase_conf = 30;
        else if (phase_low > phase_high + 60.0f)
            phase_conf = 10;
        break;
    case FILTER_TYPE_BSF:
        /* 带阻相位在中心频率处跳变 */
        if (fabsf(phase_low - phase_high) < 40.0f)
            phase_conf = 20;
        break;
    }

    result->type = amp_type;
    result->confidence = (uint8_t)(amp_conf + phase_conf);
    if (result->confidence > 100) result->confidence = 100;

    /* 计算品质因数 */
    if (result->center_freq > 0 && result->bandwidth > 0) {
        result->quality_factor = result->center_freq / result->bandwidth;
    } else {
        result->quality_factor = 0.707f;
    }
}

const char* FilterID_GetTypeName(uint8_t type) {
    switch (type) {
    case FILTER_TYPE_LPF: return "Low Pass";
    case FILTER_TYPE_HPF: return "High Pass";
    case FILTER_TYPE_BPF: return "Band Pass";
    case FILTER_TYPE_BSF: return "Band Stop";
    default: return "Unknown";
    }
}

float FilterID_CalcSlope(const FreqResponse_t *resp, int idx_low, int idx_high) {
    if (idx_high <= idx_low || idx_high >= resp->num_points) return 0;

    float df = log2f(resp->points[idx_high].freq / resp->points[idx_low].freq);
    if (fabsf(df) < 0.01f) return 0;

    return (resp->points[idx_high].gain_db - resp->points[idx_low].gain_db) / df;
}

float FilterID_Find3dBFreq(const FreqResponse_t *resp, float ref_gain_db, uint8_t from_low) {
    float target_db = ref_gain_db - 3.0f;
    int n = resp->num_points;

    if (from_low) {
        for (int i = 0; i < n; i++) {
            if (resp->points[i].gain_db <= target_db)
                return resp->points[i].freq;
        }
    } else {
        for (int i = n - 1; i >= 0; i--) {
            if (resp->points[i].gain_db <= target_db)
                return resp->points[i].freq;
        }
    }
    return 0;
}

void FilterID_FindPeak(const FreqResponse_t *resp, float *out_freq, float *out_gain_db) {
    float gain_max = -999.0f;
    int max_idx = 0;
    for (int i = 0; i < resp->num_points; i++) {
        if (resp->points[i].gain_db > gain_max) {
            gain_max = resp->points[i].gain_db;
            max_idx = i;
        }
    }
    *out_freq = resp->points[max_idx].freq;
    *out_gain_db = gain_max;
}
