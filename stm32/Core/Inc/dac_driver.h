/**
 * @file    dac_driver.h
 * @brief   DAC波形输出驱动 - STM32F407 12-bit DAC + DMA
 * @note    DAC_OUT1 = PA4, 经OPA2134缓冲后输出
 *          用于波形复刻: 将补偿后的数字波形转为模拟信号
 *
 * 信号链:
 *   数字波形计算 → DAC DMA → PA4 → OPA2134(缓冲+偏置) → 输出端
 */

#ifndef __DAC_DRIVER_H
#define __DAC_DRIVER_H

#include "main.h"

/* ======================== DAC配置 ======================== */
#define DAC_VREF_INT         3.3f        /* 内部参考电压 */
#define DAC_DMA_BUF_SIZE     512         /* DMA传输缓冲区 */

/* ======================== 函数声明 ======================== */

/**
 * @brief  初始化DAC + DMA
 * @note   配置DAC1_CH1(PA4), TIM6触发, DMA循环模式
 */
void DAC_Driver_Init(void);

/**
 * @brief  设置DAC输出直流电平
 * @param  voltage: 输出电压(V), 0~3.3
 */
void DAC_SetDC(float voltage);

/**
 * @brief  启动DMA波形输出
 * @param  buf: 波形数据缓冲区 (0~4095)
 * @param  len: 缓冲区长度
 */
void DAC_StartWaveform(const uint16_t *buf, uint16_t len);

/**
 * @brief  停止DMA波形输出
 */
void DAC_StopWaveform(void);

/**
 * @brief  生成正弦波查找表
 * @param  table: 输出查找表
 * @param  size: 表大小
 * @param  vpp: 峰峰值(V)
 * @param  offset: 直流偏置(V)
 */
void DAC_GenSineTable(uint16_t *table, uint16_t size, float vpp, float offset);

/**
 * @brief  生成矩形波查找表
 * @param  table: 输出查找表
 * @param  size: 表大小
 * @param  vpp: 峰峰值(V)
 * @param  duty: 占空比(0~1)
 * @param  offset: 直流偏置(V)
 */
void DAC_GenSquareTable(uint16_t *table, uint16_t size, float vpp, float duty, float offset);

/**
 * @brief  生成三角波查找表
 * @param  table: 输出查找表
 * @param  size: 表大小
 * @param  vpp: 峰峰值(V)
 * @param  offset: 直流偏置(V)
 */
void DAC_GenTriangleTable(uint16_t *table, uint16_t size, float vpp, float offset);

/**
 * @brief  生成补偿后的波形(应用逆滤波器)
 * @param  table: 输出查找表
 * @param  size: 表大小
 * @param  type: 波形类型(WAVE_TYPE_SINE/SQUARE/TRIANGLE)
 * @param  freq: 基波频率(Hz)
 * @param  vpp: 峰峰值(V)
 * @param  duty: 占空比(矩形波)
 * @param  params: RLC电路参数(用于逆滤波补偿)
 */
void DAC_GenCompensatedWave(uint16_t *table, uint16_t size,
                            uint8_t type, float freq, float vpp, float duty,
                            const void *params);

/**
 * @brief  DAC DMA半传输完成回调
 */
void DAC_HalfTransferCallback(void);

/**
 * @brief  DAC DMA传输完成回调
 */
void DAC_TransferCompleteCallback(void);

#endif /* __DAC_DRIVER_H */
