/**
 * @file    dac_driver.c
 * @brief   DAC波形输出驱动实现 - STM32F407 DAC1(PA4) + DMA + TIM6触发
 * @note    用于波形复刻: 补偿后波形经DAC输出
 */

#include "main.h"
#include "dac_driver.h"

extern DAC_HandleTypeDef hdac;
extern TIM_HandleTypeDef htim6;
extern DMA_HandleTypeDef hdma_dac1;

static uint16_t g_wave_buf[DAC_DMA_BUF_SIZE];

/* ── 公开函数 ── */
void DAC_Driver_Init(void) {
    HAL_DAC_Start(&hdac, DAC_CHANNEL_1);
    DAC_SetDC(1.65f);  /* 默认偏置到中点 */
}

void DAC_SetDC(float voltage) {
    if (voltage > DAC_VREF) voltage = DAC_VREF;
    if (voltage < 0) voltage = 0;

    uint32_t dac_val = (uint32_t)(voltage / DAC_VREF * 4095.0f);
    HAL_DAC_SetValue(&hdac, DAC_CHANNEL_1, DAC_ALIGN_12B_R, dac_val);
}

void DAC_StartWaveform(const uint16_t *buf, uint16_t len) {
    if (len > DAC_DMA_BUF_SIZE) len = DAC_DMA_BUF_SIZE;
    for (uint16_t i = 0; i < len; i++) g_wave_buf[i] = buf[i];

    HAL_TIM_Base_Start(&htim6);
    HAL_DAC_Start_DMA(&hdac, DAC_CHANNEL_1, (uint32_t *)g_wave_buf, len, DAC_ALIGN_12B_R);
}

void DAC_StopWaveform(void) {
    HAL_DAC_Stop_DMA(&hdac, DAC_CHANNEL_1);
    HAL_TIM_Base_Stop(&htim6);
}

/* ── 波形表生成 ── */
void DAC_GenSineTable(uint16_t *table, uint16_t size, float vpp, float offset) {
    float amp = vpp / 2.0f;
    if (amp > DAC_VREF / 2.0f) amp = DAC_VREF / 2.0f;
    for (uint16_t i = 0; i < size; i++) {
        float val = offset + amp * sinf(2.0f * M_PI * i / size);
        if (val > DAC_VREF) val = DAC_VREF;
        if (val < 0) val = 0;
        table[i] = (uint16_t)(val / DAC_VREF * 4095.0f);
    }
}

void DAC_GenSquareTable(uint16_t *table, uint16_t size, float vpp, float duty, float offset) {
    float amp = vpp / 2.0f;
    uint16_t high_count = (uint16_t)(size * duty);

    for (uint16_t i = 0; i < size; i++) {
        float val = offset + (i < high_count ? amp : -amp);
        if (val > DAC_VREF) val = DAC_VREF;
        if (val < 0) val = 0;
        table[i] = (uint16_t)(val / DAC_VREF * 4095.0f);
    }
}

void DAC_GenTriangleTable(uint16_t *table, uint16_t size, float vpp, float offset) {
    float amp = vpp / 2.0f;
    for (uint16_t i = 0; i < size; i++) {
        float phase = (float)i / size;
        float tri;
        if (phase < 0.25f)
            tri = 4.0f * phase;
        else if (phase < 0.75f)
            tri = 2.0f - 4.0f * phase;
        else
            tri = 4.0f * phase - 4.0f;

        float val = offset + amp * tri;
        if (val > DAC_VREF) val = DAC_VREF;
        if (val < 0) val = 0;
        table[i] = (uint16_t)(val / DAC_VREF * 4095.0f);
    }
}

/* ── HAL回调 ── */
void HAL_DAC_ConvHalfCpltCallbackCh1(DAC_HandleTypeDef *hdac) {
    (void)hdac;
    DAC_HalfTransferCallback();
}

void HAL_DAC_ConvCpltCallbackCh1(DAC_HandleTypeDef *hdac) {
    (void)hdac;
    DAC_TransferCompleteCallback();
}

/* 弱回调(用户可在main.c中重写) */
__weak void DAC_HalfTransferCallback(void) {}
__weak void DAC_TransferCompleteCallback(void) {}
