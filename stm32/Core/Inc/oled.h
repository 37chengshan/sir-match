/**
 * @file    oled.h
 * @brief   SSD1306 OLED显示驱动 (I2C, 128x64)
 * @note    0.96寸OLED, I2C地址 0x78
 *          支持中英文字符显示(字库编码)
 *          支持数字/浮点数显示
 *
 * 接线:
 *   PB6 -> SCL
 *   PB7 -> SDA
 */

#ifndef __OLED_H
#define __OLED_H

#include "main.h"

/* ======================== SSD1306命令定义 ======================== */
#define SSD1306_CMD_SET_MUX        0xA8
#define SSD1306_CMD_SET_OFFSET     0xD3
#define SSD1306_CMD_SET_STARTLINE  0x40
#define SSD1306_CMD_SEG_REMAP      0xA1
#define SSD1306_CMD_COM_SCAN_DEC   0xC8
#define SSD1306_CMD_SET_COMPINS    0xDA
#define SSD1306_CMD_SET_CONTRAST   0x81
#define SSD1306_CMD_PRECHARGE      0xD9
#define SSD1306_CMD_VCOMH_DESELECT 0xDB
#define SSD1306_CMD_DISPLAY_ON     0xAF
#define SSD1306_CMD_DISPLAY_OFF    0xAE
#define SSD1306_CMD_NORMAL_DISPLAY 0xA6
#define SSD1306_CMD_INVERT_DISPLAY 0xA7
#define SSD1306_CMD_CHARGE_PUMP    0x8D
#define SSD1306_CMD_SET_CLOCK      0xD5
#define SSD1306_CMD_MEMORY_MODE    0x20
#define SSD1306_CMD_SET_PAGE       0xB0
#define SSD1306_CMD_SET_COL_LOW    0x00
#define SSD1306_CMD_SET_COL_HIGH   0x10

/* 控制字节 */
#define SSD1306_CONTROL_CMD        0x00
#define SSD1306_CONTROL_DATA       0x40

/* ======================== 显示缓冲区 ======================== */
#define OLED_BUF_SIZE       (OLED_WIDTH * OLED_HEIGHT / 8)  /* 1024 bytes */

/* ======================== 字体定义 ======================== */
#define FONT_WIDTH_6x8      6
#define FONT_WIDTH_8x16     8
#define FONT_HEIGHT_8       8
#define FONT_HEIGHT_16      16

/* ======================== 函数声明 ======================== */

/**
 * @brief  初始化OLED (I2C + SSD1306配置)
 */
void OLED_Init(void);

/**
 * @brief  清屏(缓冲区清零)
 */
void OLED_Clear(void);

/**
 * @brief  刷新显示(将缓冲区发送到OLED)
 */
void OLED_Refresh(void);

/**
 * @brief  设置像素点
 * @param  x: X坐标 (0~127)
 * @param  y: Y坐标 (0~63)
 * @param  color: 1=亮, 0=灭
 */
void OLED_SetPixel(uint8_t x, uint8_t y, uint8_t color);

/**
 * @brief  画线
 * @param  x0,y0: 起点
 * @param  x1,y1: 终点
 * @param  color: 1=亮, 0=灭
 */
void OLED_DrawLine(uint8_t x0, uint8_t y0, uint8_t x1, uint8_t y1, uint8_t color);

/**
 * @brief  画矩形
 * @param  x,y: 左上角
 * @param  w,h: 宽和高
 * @param  color: 1=亮, 0=灭
 */
void OLED_DrawRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);

/**
 * @brief  填充矩形
 */
void OLED_FillRect(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t color);

/**
 * @brief  显示ASCII字符 (6x8字体)
 * @param  x: 起始X坐标
 * @param  y: 起始Y坐标(页地址 0~7)
 * @param  ch: ASCII字符
 */
void OLED_ShowChar6x8(uint8_t x, uint8_t y, char ch);

/**
 * @brief  显示ASCII字符 (8x16字体)
 * @param  x: 起始X坐标
 * @param  y: 起始Y坐标(页地址 0~7)
 * @param  ch: ASCII字符
 */
void OLED_ShowChar8x16(uint8_t x, uint8_t y, char ch);

/**
 * @brief  显示字符串 (6x8字体)
 * @param  x: 起始X坐标
 * @param  y: 页地址(0~7)
 * @param  str: 字符串
 */
void OLED_ShowString(uint8_t x, uint8_t y, const char *str);

/**
 * @brief  显示字符串 (8x16字体)
 */
void OLED_ShowString16(uint8_t x, uint8_t y, const char *str);

/**
 * @brief  显示整数
 * @param  x,y: 坐标
 * @param  num: 整数值
 * @param  len: 显示位数(补空格)
 */
void OLED_ShowInt(uint8_t x, uint8_t y, int32_t num, uint8_t len);

/**
 * @brief  显示浮点数
 * @param  x,y: 坐标
 * @param  num: 浮点值
 * @param  int_len: 整数部分位数
 * @param  dec_len: 小数部分位数
 */
void OLED_ShowFloat(uint8_t x, uint8_t y, float num, uint8_t int_len, uint8_t dec_len);

/**
 * @brief  显示波形(在缓冲区绘制波形)
 * @param  data: 数据数组(归一化到0~1)
 * @param  len: 数据长度
 * @param  x_start: 起始X
 * @param  y_start: 起始Y(顶部)
 * @param  width: 绘制宽度
 * @param  height: 绘制高度
 */
void OLED_ShowWaveform(const float *data, uint16_t len,
                       uint8_t x_start, uint8_t y_start,
                       uint8_t width, uint8_t height);

/**
 * @brief  显示频率响应曲线
 * @param  freqs: 频率数组
 * @param  gains_db: 增益数组(dB)
 * @param  len: 数据点数
 * @param  x,y: 绘图区域左上角
 * @param  w,h: 绘图区域宽高
 */
void OLED_ShowBodePlot(const float *freqs, const float *gains_db, uint16_t len,
                       uint8_t x, uint8_t y, uint8_t w, uint8_t h);

/**
 * @brief  显示进度条
 * @param  x,y: 左上角
 * @param  w,h: 宽高
 * @param  progress: 进度(0~100)
 */
void OLED_ShowProgressBar(uint8_t x, uint8_t y, uint8_t w, uint8_t h, uint8_t progress);

/**
 * @brief  反转显示区域
 */
void OLED_InvertArea(uint8_t x, uint8_t y, uint8_t w, uint8_t h);

/**
 * @brief  设置对比度
 * @param  contrast: 0~255
 */
void OLED_SetContrast(uint8_t contrast);

/**
 * @brief  显示开关
 * @param  on: 1=开, 0=关
 */
void OLED_DisplayOn(uint8_t on);

#endif /* __OLED_H */
