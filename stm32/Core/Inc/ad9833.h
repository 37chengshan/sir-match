/**
 * @file    ad9833.h
 * @brief   AD9833 DDS信号发生器驱动
 * @note    SPI接口, 支持正弦/三角/方波输出
 *          MCLK = 25MHz (模块板载晶振)
 *          频率分辨率 = 25MHz / 2^28 ≈ 0.093Hz
 *
 * 接线:
 *   PA5  -> SCLK
 *   PA7  -> SDATA (MOSI)
 *   PA4  -> FSYNC (CS)
 */

#ifndef __AD9833_H
#define __AD9833_H

#include "main.h"

/* ======================== AD9833寄存器定义 ======================== */

/* 控制寄存器位 */
#define AD9833_CTRL_B28     (1 << 13)   /* 28位频率字分两次写入 */
#define AD9833_CTRL_HLB     (1 << 12)   /* 高/低14位选择 */
#define AD9833_CTRL_FSELECT (1 << 11)   /* 频率寄存器选择 */
#define AD9833_CTRL_PSELECT (1 << 10)   /* 相位寄存器选择 */
#define AD9833_CTRL_RESET   (1 << 8)    /* 复位 */
#define AD9833_CTRL_SLEEP1  (1 << 7)    /* DAC休眠 */
#define AD9833_CTRL_SLEEP12 (1 << 6)    /* 内部时钟禁用 */
#define AD9833_CTRL_OPBITEN (1 << 5)    /* 输出使能 */
#define AD9833_CTRL_DIV2    (1 << 3)    /* 方波分频 */
#define AD9833_CTRL_MODE    (1 << 1)    /* 三角波模式 */

/* 波形选择 */
#define AD9833_WAVE_SINE     0
#define AD9833_WAVE_TRIANGLE 1
#define AD9833_WAVE_SQUARE   2

/* 频率/相位寄存器地址 */
#define AD9833_FREQ0_REG    0x4000
#define AD9833_FREQ1_REG    0x8000
#define AD9833_PHASE0_REG   0xC000
#define AD9833_PHASE1_REG   0xE000

/* MCLK频率 */
#define AD9833_MCLK         25000000UL

/* FSYNC引脚 */
#define AD9833_FSYNC_PIN    GPIO_PIN_4
#define AD9833_FSYNC_PORT   GPIOA

/* ======================== 函数声明 ======================== */

/**
 * @brief  初始化AD9833
 * @note   配置SPI, 发送复位命令
 */
void AD9833_Init(void);

/**
 * @brief  设置输出频率
 * @param  freq_hz: 目标频率(Hz), 范围 0 ~ 12.5MHz
 */
void AD9833_SetFrequency(uint32_t freq_hz);

/**
 * @brief  设置输出波形类型
 * @param  wave_type: AD9833_WAVE_SINE / _TRIANGLE / _SQUARE
 */
void AD9833_SetWaveform(uint8_t wave_type);

/**
 * @brief  设置输出相位
 * @param  phase_deg: 相位(度), 0~360
 */
void AD9833_SetPhase(float phase_deg);

/**
 * @brief  复位AD9833 (输出停止)
 */
void AD9833_Reset(void);

/**
 * @brief  启动输出
 */
void AD9833_Start(void);

/**
 * @brief  写16位数据到AD9833
 * @param  data: 16位控制字
 */
void AD9833_Write16(uint16_t data);

#endif /* __AD9833_H */
