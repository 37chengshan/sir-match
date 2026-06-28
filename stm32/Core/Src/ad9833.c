/**
 * @file    ad9833.c
 * @brief   AD9833 DDS信号发生器驱动实现
 * @note    SPI1: PA5(SCK), PA7(MOSI), PB2(FSYNC)
 *          MCLK = 25MHz, 频率分辨率 ≈ 0.093Hz
 */

#include "main.h"

/* 外部SPI句柄 */
extern SPI_HandleTypeDef hspi1;

/* 控制寄存器缓存(用于快速切换) */
static uint16_t g_ctrl_reg = 0;

/* SPI1引脚 */
#define FSYNC_PORT  GPIOB
#define FSYNC_PIN   GPIO_PIN_2

#define FSYNC_LOW()   HAL_GPIO_WritePin(FSYNC_PORT, FSYNC_PIN, GPIO_PIN_RESET)
#define FSYNC_HIGH()  HAL_GPIO_WritePin(FSYNC_PORT, FSYNC_PIN, GPIO_PIN_SET)

/* ── 内部函数 ── */
static void AD9833_SPI_Write(uint16_t data) {
    FSYNC_LOW();
    HAL_SPI_Transmit(&hspi1, (uint8_t *)&data, 1, HAL_MAX_DELAY);
    FSYNC_HIGH();
}

/* ── 公开函数 ── */
void AD9833_Init(void) {
    /* 拉高FSYNC */
    FSYNC_HIGH();

    /* 复位AD9833 */
    g_ctrl_reg = AD9833_CTRL_RESET;
    AD9833_SPI_Write(g_ctrl_reg);

    /* 延时等待复位 */
    HAL_Delay(1);

    /* 配置: B28(连续频率写入), 正弦波 */
    g_ctrl_reg = AD9833_CTRL_B28;
    AD9833_SPI_Write(g_ctrl_reg);

    /* 设置默认频率 1kHz */
    AD9833_SetFrequency(1000);
}

void AD9833_SetFrequency(uint32_t freq_hz) {
    if (freq_hz > 12500000) freq_hz = 12500000;
    if (freq_hz < 1) freq_hz = 1;

    /* 计算频率字: freq_word = freq * 2^28 / MCLK */
    uint32_t freq_word = (uint32_t)(((uint64_t)freq_hz << 28) / AD9833_MCLK);

    /* 写FREQ0寄存器: 先低14位, 再高14位 */
    uint16_t lsb = (uint16_t)(freq_word & 0x3FFF);
    uint16_t msb = (uint16_t)((freq_word >> 14) & 0x3FFF);

    AD9833_SPI_Write(AD9833_FREQ0_REG | lsb);
    AD9833_SPI_Write(AD9833_FREQ0_REG | msb);
}

void AD9833_SetWaveform(uint8_t wave_type) {
    /* 清除波形选择位 */
    g_ctrl_reg &= ~(AD9833_CTRL_MODE | AD9833_CTRL_OPBITEN | AD9833_CTRL_DIV2);

    switch (wave_type) {
    case AD9833_WAVE_SINE:
        /* 正弦: 默认(无额外位) */
        break;
    case AD9833_WAVE_TRIANGLE:
        g_ctrl_reg |= AD9833_CTRL_MODE;
        break;
    case AD9833_WAVE_SQUARE:
        /* 方波: OPBITEN + DIV2 */
        g_ctrl_reg |= (AD9833_CTRL_OPBITEN | AD9833_CTRL_DIV2);
        break;
    default:
        break;
    }

    AD9833_SPI_Write(g_ctrl_reg);
}

void AD9833_SetPhase(float phase_deg) {
    /* 相位字: phase_word = phase_deg * 4096 / 360 */
    uint16_t phase_word = (uint16_t)(phase_deg * 4096.0f / 360.0f);
    phase_word &= 0x0FFF;
    AD9833_SPI_Write(AD9833_PHASE0_REG | phase_word);
}

void AD9833_Reset(void) {
    g_ctrl_reg |= AD9833_CTRL_RESET;
    AD9833_SPI_Write(g_ctrl_reg);
}

void AD9833_Start(void) {
    g_ctrl_reg &= ~AD9833_CTRL_RESET;
    AD9833_SPI_Write(g_ctrl_reg);
}

void AD9833_Write16(uint16_t data) {
    AD9833_SPI_Write(data);
}
