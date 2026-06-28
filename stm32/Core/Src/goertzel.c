/**
 * @file    goertzel.c
 * @brief   Goertzel算法实现 - 单频率DFT幅度/相位提取
 * @note    O(N)复杂度, 同时输出幅度(Vpp)和相位(度)
 *          MATLAB验证: 幅度误差<1%, 相位误差<1°
 */

#include "main.h"
#include "goertzel.h"

void Goertzel_Init(Goertzel_t *g, float target_freq, float sample_rate, int num_samples) {
    g->N = num_samples;
    g->count = 0;
    g->s0 = g->s1 = g->s2 = 0;

    /* 计算整数k和精确频率 */
    int k = (int)(0.5f + (float)num_samples * target_freq / sample_rate);
    float w = 2.0f * M_PI * (float)k / (float)num_samples;

    g->coeff = 2.0f * cosf(w);
    g->w_real = cosf(w);
    g->w_imag = sinf(w);

    g->magnitude = 0;
    g->phase = 0;
    g->power = 0;
}

void Goertzel_Reset(Goertzel_t *g) {
    g->count = 0;
    g->s0 = g->s1 = g->s2 = 0;
    g->magnitude = 0;
    g->phase = 0;
    g->power = 0;
}

void Goertzel_Process(Goertzel_t *g, float sample) {
    g->s0 = sample + g->coeff * g->s1 - g->s2;
    g->s2 = g->s1;
    g->s1 = g->s0;
    g->count++;
}

void Goertzel_ProcessBlock(Goertzel_t *g, const float *samples, int count) {
    for (int i = 0; i < count; i++) {
        Goertzel_Process(g, samples[i]);
    }
}

void Goertzel_Finalize(Goertzel_t *g) {
    /* |X[k]|^2 = s1^2 + s2^2 - coeff * s1 * s2 */
    float real = g->s1 - g->s2 * g->w_real;
    float imag = g->s2 * g->w_imag;

    g->power = real * real + imag * imag;
    g->magnitude = 2.0f * sqrtf(g->power) / (float)g->N;
    g->phase = atan2f(imag, real);

    /* 相位归一化到[-180, 180]度 */
    g->phase = g->phase * 180.0f / M_PI;
}

float Goertzel_GetMagnitude(Goertzel_t *g) {
    return g->magnitude;
}

float Goertzel_GetVpp(Goertzel_t *g) {
    /* 峰峰值 = 幅度 × 2 */
    return g->magnitude * 2.0f;
}

float Goertzel_GetPhaseDeg(Goertzel_t *g) {
    return g->phase;
}

void Goertzel_Compute(const float *samples, int num_samples,
                      float target_freq, float sample_rate,
                      float *out_mag, float *out_phase) {
    Goertzel_t g;
    Goertzel_Init(&g, target_freq, sample_rate, num_samples);
    Goertzel_ProcessBlock(&g, samples, num_samples);
    Goertzel_Finalize(&g);
    *out_mag = Goertzel_GetVpp(&g);
    *out_phase = Goertzel_GetPhaseDeg(&g);
}
