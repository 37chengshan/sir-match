/**
 * @file    waveform.c
 * @brief   波形复刻实现 - 逆滤波器补偿 + DAC输出
 * @note    基于FFT谐波分析+频域逆滤波补偿
 *          补偿后波形通过DAC直接输出
 */

#include "main.h"
#include "waveform.h"
#include "dac_driver.h"

/* 全局复刻器实例 */
static WaveformReplicator_t g_replicator;

/* ── 内部函数 ── */
static float rlc_gain_at_freq(const RLC_Params_t *params, float freq) {
    float w = 2.0f * M_PI * freq;
    float LC = params->L * params->C;
    float RC = params->R * params->C;
    float den = 1.0f - w * w * LC;
    float img = w * RC;
    return 1.0f / sqrtf(den * den + img * img);
}

static float rlc_phase_at_freq(const RLC_Params_t *params, float freq) {
    float w = 2.0f * M_PI * freq;
    float LC = params->L * params->C;
    float RC = params->R * params->C;
    float img = -w * RC;
    float den = 1.0f - w * w * LC;
    return atan2f(img, den) * 180.0f / M_PI;
}

/* ── 公开函数 ── */
void Waveform_Init(WaveformReplicator_t *wr) {
    memset(wr, 0, sizeof(WaveformReplicator_t));
    wr->ready = 0;
}

void Waveform_SetSine(WaveformReplicator_t *wr, float freq, float vpp) {
    wr->target.wave_type = WAVE_TYPE_SINE;
    wr->target.fundamental_freq = freq;
    wr->target.total_vpp = vpp;
    wr->target.num_harmonics = 1;
    wr->target.harmonics[0].freq = freq;
    wr->target.harmonics[0].amplitude = vpp;
    wr->target.harmonics[0].phase = 0;
    wr->target.dc_offset = 0;
    wr->ready = 0;
}

void Waveform_SetSquare(WaveformReplicator_t *wr, float freq, float vpp, float duty) {
    wr->target.wave_type = WAVE_TYPE_SQUARE;
    wr->target.fundamental_freq = freq;
    wr->target.total_vpp = vpp;
    wr->target.dc_offset = vpp * (duty - 0.5f);

    /* 矩形波傅里叶级数: 奇次谐波, 幅度 4A/(nπ) */
    float A = vpp / 2.0f;
    int n_harm = Waveform_SquareHarmonics(freq, vpp, duty,
                                          wr->target.harmonics, MAX_HARMONICS);
    wr->target.num_harmonics = n_harm;
    wr->ready = 0;
}

void Waveform_SetTriangle(WaveformReplicator_t *wr, float freq, float vpp) {
    wr->target.wave_type = WAVE_TYPE_TRIANGLE;
    wr->target.fundamental_freq = freq;
    wr->target.total_vpp = vpp;
    wr->target.dc_offset = 0;

    int n_harm = Waveform_TriangleHarmonics(freq, vpp,
                                            wr->target.harmonics, MAX_HARMONICS);
    wr->target.num_harmonics = n_harm;
    wr->ready = 0;
}

void Waveform_Compensate(WaveformReplicator_t *wr) {
    if (!wr->circuit_params || !wr->circuit_params->valid) return;

    RLC_Params_t *p = wr->circuit_params;

    /* 对每个谐波分量应用逆滤波器补偿 */
    for (int i = 0; i < wr->target.num_harmonics && i < MAX_HARMONICS; i++) {
        float freq = wr->target.harmonics[i].freq;

        /* 计算电路在此频率的增益和相位 */
        float gain_circuit = rlc_gain_at_freq(p, freq);
        float phase_circuit = rlc_phase_at_freq(p, freq);

        /* 逆滤波: 补偿增益衰减和相位偏移 */
        float inv_gain = 1.0f / (gain_circuit + 0.01f);  /* 正则化 */
        if (inv_gain > 10.0f) inv_gain = 10.0f;           /* 增益限幅 */

        wr->compensated.harmonics[i].freq = freq;
        wr->compensated.harmonics[i].amplitude =
            wr->target.harmonics[i].amplitude * inv_gain;
        wr->compensated.harmonics[i].phase =
            wr->target.harmonics[i].phase - phase_circuit;

        /* 相位归一化 */
        while (wr->compensated.harmonics[i].phase > 180.0f)
            wr->compensated.harmonics[i].phase -= 360.0f;
        while (wr->compensated.harmonics[i].phase < -180.0f)
            wr->compensated.harmonics[i].phase += 360.0f;
    }

    wr->compensated.num_harmonics = wr->target.num_harmonics;
    wr->compensated.fundamental_freq = wr->target.fundamental_freq;
    wr->compensated.dc_offset = wr->target.dc_offset * inv_gain;
}

void Waveform_GenerateTable(WaveformReplicator_t *wr) {
    for (uint16_t i = 0; i < WAVEFORM_TABLE_SIZE; i++) {
        float phase = (float)i / WAVEFORM_TABLE_SIZE * 2.0f * M_PI;
        float val = wr->compensated.dc_offset;

        for (int h = 0; h < wr->compensated.num_harmonics && h < MAX_HARMONICS; h++) {
            float harmonic_phase = (float)(h * 2 + 1) * phase;  /* 奇次谐波 */
            float amp = wr->compensated.harmonics[h].amplitude;
            float phi = wr->compensated.harmonics[h].phase * M_PI / 180.0f;
            val += amp * cosf(harmonic_phase + phi);
        }

        /* 映射到DAC范围 [0, 4095] */
        if (val > DAC_VREF) val = DAC_VREF;
        if (val < 0) val = 0;
        wr->output_table[i] = (uint16_t)(val / DAC_VREF * 4095.0f);
    }
    wr->ready = 1;
}

float Waveform_GetOutput(WaveformReplicator_t *wr, uint16_t index) {
    if (index >= WAVEFORM_TABLE_SIZE) index = 0;
    return wr->output_table[index];
}

int Waveform_SquareHarmonics(float freq, float vpp, float duty,
                              Harmonic_t *harmonics, int max_harmonics) {
    int n = 0;
    float A = vpp / 2.0f;

    for (int k = 1; k <= max_harmonics * 2 && n < max_harmonics; k += 2) {
        float harm_freq = freq * k;
        if (harm_freq > 50000.0f) break;

        harmonics[n].freq = harm_freq;
        /* 矩形波傅里叶系数: 4A/(kπ) * sin(k*π*duty) */
        harmonics[n].amplitude = (4.0f * A) / (k * M_PI) * sinf(k * M_PI * duty);
        if (harmonics[n].amplitude < 0) harmonics[n].amplitude = -harmonics[n].amplitude;
        harmonics[n].phase = 0;
        n++;
    }
    return n;
}

int Waveform_TriangleHarmonics(float freq, float vpp,
                                Harmonic_t *harmonics, int max_harmonics) {
    int n = 0;
    float A = vpp / 2.0f;
    int sign = 1;

    for (int k = 1; k <= max_harmonics * 2 && n < max_harmonics; k += 2) {
        float harm_freq = freq * k;
        if (harm_freq > 50000.0f) break;

        harmonics[n].freq = harm_freq;
        /* 三角波傅里叶系数: 8A/(k²π²), 交替符号 */
        harmonics[n].amplitude = (8.0f * A) / (k * k * M_PI * M_PI);
        harmonics[n].phase = (sign < 0) ? 180.0f : 0;
        harmonics[n].amplitude = fabsf(harmonics[n].amplitude);
        n++;
        sign = -sign;
    }
    return n;
}

void Waveform_OutputViaDDS(WaveformReplicator_t *wr) {
    if (!wr->ready) return;

    /* 通过DAC输出补偿后的波形表 */
    DAC_StartWaveform(wr->output_table, WAVEFORM_TABLE_SIZE);
}

float Waveform_EvaluateQuality(WaveformReplicator_t *wr) {
    if (!wr->ready) return 100.0f;

    /* 计算补偿后的频率响应与目标的偏差 */
    float error = 0;
    int count = 0;

    for (int i = 0; i < wr->compensated.num_harmonics && i < MAX_HARMONICS; i++) {
        float target_amp = wr->target.harmonics[i].amplitude;
        float comp_amp = wr->compensated.harmonics[i].amplitude;

        if (target_amp > 0.001f) {
            error += fabsf(comp_amp - target_amp) / target_amp;
            count++;
        }
    }

    return (count > 0) ? (error / count * 100.0f) : 0;
}
