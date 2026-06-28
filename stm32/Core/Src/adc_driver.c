/**
 * @file    adc_driver.c
 * @brief   ADC采集驱动实现 - STM32F407 ADC1 + DMA + TIM2触发
 * @note    PA0(CH0)=信号输入, PA1(CH1)=参考电压
 *          双通道DMA循环采集, 半传输/全传输中断
 */

#include "main.h"

extern ADC_HandleTypeDef hadc1;
extern TIM_HandleTypeDef htim2;
extern DMA_HandleTypeDef hdma_adc1;

/* DMA缓冲区(双通道交替) */
static uint16_t adc_dma_buf[ADC_BUF_SIZE * 2];  /* CH0/CH1交替 */
static volatile uint8_t g_data_ready = 0;
static volatile uint16_t g_half_buf_size = ADC_BUF_SIZE;  /* 单通道半传输大小 */

/* ── 公开函数 ── */
void ADC_Driver_Init(void) {
    /* 校准ADC */
    HAL_ADCEx_Calibration_Start(&hadc1);

    /* 启动ADC DMA采集 */
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_dma_buf, ADC_BUF_SIZE * 2);

    /* TIM2触发ADC (控制采样率) */
    HAL_TIM_Base_Start(&htim2);

    g_data_ready = 0;
}

void ADC_StartContinuous(void) {
    HAL_ADC_Start_DMA(&hadc1, (uint32_t *)adc_dma_buf, ADC_BUF_SIZE * 2);
    g_data_ready = 0;
}

void ADC_StopContinuous(void) {
    HAL_ADC_Stop_DMA(&hadc1);
    g_data_ready = 0;
}

uint16_t ADC_ReadSingle(uint32_t channel) {
    ADC_ChannelConfTypeDef sConfig = {0};
    sConfig.Channel = channel;
    sConfig.Rank = 1;
    sConfig.SamplingTime = ADC_SAMPLETIME_480CYCLES;
    HAL_ADC_ConfigChannel(&hadc1, &sConfig);

    HAL_ADC_Start(&hadc1);
    HAL_ADC_PollForConversion(&hadc1, 10);
    uint16_t val = HAL_ADC_GetValue(&hadc1);
    HAL_ADC_Stop(&hadc1);
    return val;
}

uint16_t ADC_GetSamples(float *buf, uint16_t count, uint8_t channel) {
    if (count > ADC_BUF_SIZE) count = ADC_BUF_SIZE;
    if (channel > 1) return 0;

    uint16_t actual = 0;
    for (uint16_t i = 0; i < count; i++) {
        uint16_t raw = adc_dma_buf[i * 2 + channel];
        buf[i] = ADC_TO_VOLTAGE(raw);
        actual++;
    }
    return actual;
}

void ADC_Measure(ADC_Measure_t *result, uint16_t num_samples) {
    if (num_samples == 0 || num_samples > ADC_BUF_SIZE) num_samples = ADC_BUF_SIZE;

    uint32_t sum = 0;
    uint32_t peak = 0;
    uint32_t valley = 4095;

    for (uint16_t i = 0; i < num_samples; i++) {
        uint16_t val = adc_dma_buf[i * 2];  /* CH0 = 信号 */
        sum += val;
        if (val > peak) peak = val;
        if (val < valley) valley = val;
    }

    float avg = (float)sum / num_samples;
    result->dc_offset = ADC_TO_VOLTAGE((uint16_t)avg);
    result->raw_peak = peak;
    result->raw_valley = valley;
    result->vpp = ADC_TO_VOLTAGE(peak - valley);

    /* RMS估算 (假设正弦波) */
    result->ac_rms = result->vpp * 0.3535f;  /* Vpp / (2√2) */
    result->freq_est = 0;  /* 由Goertzel提供 */
}

void ADC_SetSampleRate(uint32_t sample_rate) {
    /* TIM2触发频率 = 84MHz / (PSC+1) / (ARR+1) */
    uint32_t arr = APB1_TIMER_FREQ / sample_rate - 1;
    if (arr > 65535) arr = 65535;
    __HAL_TIM_SET_AUTORELOAD(&htim2, (uint16_t)arr);
}

uint8_t ADC_DataReady(void) {
    return g_data_ready;
}

void ADC_ClearReady(void) {
    g_data_ready = 0;
}

/* ── DMA回调 ── */
void ADC_HalfTransferCallback(void) {
    g_data_ready = 1;
    g_half_buf_size = ADC_BUF_SIZE;
}

void ADC_TransferCompleteCallback(void) {
    g_data_ready = 1;
    g_half_buf_size = ADC_BUF_SIZE;
}

/* HAL ADC DMA半传输回调 */
void HAL_ADC_ConvHalfCpltCallback(ADC_HandleTypeDef *hadc) {
    (void)hadc;
    ADC_HalfTransferCallback();
}

/* HAL ADC DMA传输完成回调 */
void HAL_ADC_ConvCpltCallback(ADC_HandleTypeDef *hadc) {
    (void)hadc;
    ADC_TransferCompleteCallback();
}
