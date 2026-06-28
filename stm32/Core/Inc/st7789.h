/**
 * @file    st7789.h
 * @brief   ST7789V 240x240 IPS TFT 显示驱动 (SPI)
 * @note    共享SPI1总线: SCK=PA5, MOSI=PA7
 *          CS=PB0, DC=PB1, RST=PA6, BLK=PB3(TIM2_CH2 PWM)
 *          分辨率 240×240, 65K色, 最高60fps
 *
 * 接线:
 *   PA5 -> SCK  (共享)
 *   PA7 -> MOSI (共享)
 *   PB0 -> CS   (片选)
 *   PB1 -> DC   (数据/命令)
 *   PA6 -> RST  (复位)
 *   PB3 -> BLK  (背光PWM)
 */

#ifndef __ST7789_H
#define __ST7789_H

#include "main.h"

/* ======================== 屏幕参数 ======================== */
#define ST7789_WIDTH          240
#define ST7789_HEIGHT         240

/* ======================== ST7789 命令定义 ======================== */
#define ST7789_NOP            0x00
#define ST7789_SWRESET        0x01
#define ST7789_SLPIN          0x10
#define ST7789_SLPOUT         0x11
#define ST7789_NORON          0x13
#define ST7789_INVOFF         0x20
#define ST7789_INVON          0x21
#define ST7789_DISPOFF        0x28
#define ST7789_DISPON         0x29
#define ST7789_CASET          0x2A
#define ST7789_RASET          0x2B
#define ST7789_RAMWR          0x2C
#define ST7789_MADCTL         0x36
#define ST7789_COLMOD         0x3A

#define ST7789_MADCTL_MY      0x80
#define ST7789_MADCTL_MX      0x40
#define ST7789_MADCTL_MV      0x20
#define ST7789_MADCTL_ML      0x10
#define ST7789_MADCTL_RGB     0x00
#define ST7789_MADCTL_BGR     0x08

/* ======================== 引脚定义 ======================== */
#define ST7789_CS_PORT        GPIOB
#define ST7789_CS_PIN         GPIO_PIN_0
#define ST7789_DC_PORT        GPIOB
#define ST7789_DC_PIN         GPIO_PIN_1
#define ST7789_RST_PORT       GPIOA
#define ST7789_RST_PIN        GPIO_PIN_6
#define ST7789_BLK_PORT       GPIOB
#define ST7789_BLK_PIN        GPIO_PIN_3

/* 引脚控制宏 */
#define ST7789_CS_LOW()       HAL_GPIO_WritePin(ST7789_CS_PORT, ST7789_CS_PIN, GPIO_PIN_RESET)
#define ST7789_CS_HIGH()      HAL_GPIO_WritePin(ST7789_CS_PORT, ST7789_CS_PIN, GPIO_PIN_SET)
#define ST7789_DC_CMD()       HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_RESET)
#define ST7789_DC_DATA()      HAL_GPIO_WritePin(ST7789_DC_PORT, ST7789_DC_PIN, GPIO_PIN_SET)
#define ST7789_RST_LOW()      HAL_GPIO_WritePin(ST7789_RST_PORT, ST7789_RST_PIN, GPIO_PIN_RESET)
#define ST7789_RST_HIGH()     HAL_GPIO_WritePin(ST7789_RST_PORT, ST7789_RST_PIN, GPIO_PIN_SET)

/* ======================== 颜色定义 ======================== */
#define COLOR_BLACK          0x0000
#define COLOR_WHITE          0xFFFF
#define COLOR_RED            0xF800
#define COLOR_GREEN          0x07E0
#define COLOR_BLUE           0x001F
#define COLOR_CYAN           0x07FF
#define COLOR_MAGENTA        0xF81F
#define COLOR_YELLOW         0xFFE0
#define COLOR_ORANGE         0xFD20
#define COLOR_GRAY           0x8410
#define COLOR_DARK_GRAY      0x4208
#define COLOR_NAVY           0x0010

/* ======================== Bode图颜色方案 ======================== */
#define COLOR_BODE_BG        0x0C1A
#define COLOR_BODE_GRID      0x2965
#define COLOR_BODE_MAG       0x07FF  /* 青色 - 幅度曲线 */
#define COLOR_BODE_PHASE     0xFFE0  /* 黄色 - 相位曲线 */
#define COLOR_BODE_TEXT      0xFFFF
#define COLOR_BODE_CURSOR    0xF800  /* 红色 - 光标 */

/* ======================== RGB565 转换宏 ======================== */
#define RGB565(r, g, b)      ((((r) & 0xF8) << 8) | (((g) & 0xFC) << 3) | ((b) >> 3))

/* ======================== 函数声明 ======================== */

/**
 * @brief  初始化ST7789 (SPI + 命令序列)
 */
void ST7789_Init(void);

/**
 * @brief  设置背光亮度
 * @param  brightness: 0~100 (百分比)
 */
void ST7789_SetBacklight(uint8_t brightness);

/**
 * @brief  全屏填充颜色
 * @param  color: RGB565颜色值
 */
void ST7789_FillScreen(uint16_t color);

/**
 * @brief  填充矩形区域
 * @param  x, y: 左上角坐标
 * @param  w, h: 宽和高
 * @param  color: RGB565颜色值
 */
void ST7789_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

/**
 * @brief  设置绘图窗口
 * @param  x, y: 起始坐标
 * @param  w, h: 宽和高
 */
void ST7789_SetWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

/**
 * @brief  批量写入像素数据 (DMA方式)
 * @param  data: 像素数据缓冲区
 * @param  len: 数据长度(像素数)
 */
void ST7789_WritePixels(const uint16_t *data, uint16_t len);

/**
 * @brief  画点
 * @param  x, y: 坐标
 * @param  color: RGB565颜色值
 */
void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color);

/**
 * @brief  画线
 * @param  x0, y0: 起点
 * @param  x1, y1: 终点
 * @param  color: RGB565颜色值
 */
void ST7789_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color);

/**
 * @brief  画空心矩形
 * @param  x, y: 左上角
 * @param  w, h: 宽和高
 * @param  color: RGB565颜色值
 */
void ST7789_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

/**
 * @brief  显示ASCII字符 (8x16字体)
 * @param  x, y: 起始坐标
 * @param  ch: 字符
 * @param  fg_color: 前景色
 * @param  bg_color: 背景色
 */
void ST7789_DrawChar(uint16_t x, uint16_t y, char ch, uint16_t fg_color, uint16_t bg_color);

/**
 * @brief  显示字符串
 * @param  x, y: 起始坐标
 * @param  str: 字符串
 * @param  fg_color: 前景色
 * @param  bg_color: 背景色
 */
void ST7789_DrawString(uint16_t x, uint16_t y, const char *str, uint16_t fg_color, uint16_t bg_color);

/**
 * @brief  显示整数
 * @param  x, y: 坐标
 * @param  num: 整数值
 * @param  len: 显示位数(补空格)
 * @param  fg_color: 前景色
 * @param  bg_color: 背景色
 */
void ST7789_DrawInt(uint16_t x, uint16_t y, int32_t num, uint8_t len, uint16_t fg_color, uint16_t bg_color);

/**
 * @brief  显示浮点数
 * @param  x, y: 坐标
 * @param  num: 浮点值
 * @param  int_len: 整数位数
 * @param  dec_len: 小数位数
 * @param  fg_color: 前景色
 * @param  bg_color: 背景色
 */
void ST7789_DrawFloat(uint16_t x, uint16_t y, float num, uint8_t int_len, uint8_t dec_len, uint16_t fg_color, uint16_t bg_color);

/**
 * @brief  显示波形曲线
 * @param  data: 归一化数据数组(0~1)
 * @param  len: 数据长度
 * @param  x, y: 绘图区域左上角
 * @param  w, h: 绘图区域宽高
 * @param  color: 波形颜色
 */
void ST7789_DrawWaveform(const float *data, uint16_t len, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);

/**
 * @brief  显示Bode图(幅度+相位)
 * @param  freqs: 频率数组
 * @param  gains_db: 增益数组(dB)
 * @param  phases: 相位数组(度)
 * @param  len: 数据点数
 * @param  x, y: 绘图区域左上角
 * @param  w, h: 绘图区域宽高
 */
void ST7789_DrawBode(const float *freqs, const float *gains_db, const float *phases, uint16_t len, uint16_t x, uint16_t y, uint16_t w, uint16_t h);

/**
 * @brief  显示进度条
 * @param  x, y: 左上角
 * @param  w, h: 宽高
 * @param  progress: 进度(0~100)
 * @param  bar_color: 进度条颜色
 * @param  bg_color: 背景颜色
 */
void ST7789_DrawProgressBar(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t progress, uint16_t bar_color, uint16_t bg_color);

/**
 * @brief  显示标题栏 (顶部状态条)
 * @param  title: 标题文字
 */
void ST7789_DrawTitleBar(const char *title);

/**
 * @brief  显示菜单选项
 * @param  items: 菜单项字符串数组
 * @param  num_items: 菜单项数量
 * @param  selected: 当前选中项索引
 */
void ST7789_DrawMenu(const char *items[], uint8_t num_items, uint8_t selected);

/**
 * @brief  显示系统状态摘要
 * @param  mode: 当前模式名称
 * @param  freq: 频率值
 * @param  vpp: 峰峰值
 * @param  ftype: 滤波器类型名称
 */
void ST7789_DrawStatus(const char *mode, float freq, float vpp, const char *ftype);

#endif /* __ST7789_H */
