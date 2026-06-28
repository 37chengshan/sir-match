/**
 * @file    st7789.c
 * @brief   ST7789V 240x240 IPS TFT 显示驱动实现
 * @note    SPI2: PB13(SCK), PB15(MOSI), PB12(CS), PB14(DC)
 *          RST=PA6, BLK=PB3(TIM3_CH2 PWM)
 *          共享SPI2, 单独CS, 使用DMA传输
 */

#include "main.h"
#include "st7789.h"

extern SPI_HandleTypeDef hspi2;
extern TIM_HandleTypeDef htim3;

/* ── 内部函数 ── */
static void ST7789_WriteCmd(uint8_t cmd) {
    ST7789_DC_CMD();
    ST7789_CS_LOW();
    HAL_SPI_Transmit(&hspi2, &cmd, 1, HAL_MAX_DELAY);
    ST7789_CS_HIGH();
}

static void ST7789_WriteData(uint8_t data) {
    ST7789_DC_DATA();
    ST7789_CS_LOW();
    HAL_SPI_Transmit(&hspi2, &data, 1, HAL_MAX_DELAY);
    ST7789_CS_HIGH();
}

static void ST7789_WriteData16(uint16_t data) {
    ST7789_DC_DATA();
    ST7789_CS_LOW();
    uint8_t buf[2] = { (uint8_t)(data >> 8), (uint8_t)(data & 0xFF) };
    HAL_SPI_Transmit(&hspi2, buf, 2, HAL_MAX_DELAY);
    ST7789_CS_HIGH();
}

/* ── 公开函数 ── */
void ST7789_Init(void) {
    /* 硬件复位 */
    ST7789_RST_LOW();
    HAL_Delay(5);
    ST7789_RST_HIGH();
    HAL_Delay(10);

    /* CS初始高 */
    ST7789_CS_HIGH();

    /* 初始化序列 */
    ST7789_WriteCmd(0x11);   /* 退出睡眠 */
    HAL_Delay(120);

    ST7789_WriteCmd(0x36);   /* MADCTL: 内存数据访问控制 */
    ST7789_WriteData(0x00);  /* 从左到右, 从上到下, RGB顺序 */

    ST7789_WriteCmd(0x3A);   /* COLMOD: 像素格式 */
    ST7789_WriteData(0x55);  /* 16-bit RGB565 */

    ST7789_WriteCmd(0xB2);   /* PORCTRL */
    ST7789_WriteData(0x0C);
    ST7789_WriteData(0x0C);
    ST7789_WriteData(0x00);
    ST7789_WriteData(0x33);
    ST7789_WriteData(0x33);

    ST7789_WriteCmd(0xB7);   /* GCTRL */
    ST7789_WriteData(0x35);

    ST7789_WriteCmd(0xBB);   /* VCOMS */
    ST7789_WriteData(0x19);

    ST7789_WriteCmd(0xC0);   /* LCMCTRL */
    ST7789_WriteData(0x2C);

    ST7789_WriteCmd(0xC2);   /* VDVVRHEN */
    ST7789_WriteData(0x01);

    ST7789_WriteCmd(0xC3);   /* VRHS */
    ST7789_WriteData(0x12);

    ST7789_WriteCmd(0xC4);   /* VDVS */
    ST7789_WriteData(0x20);

    ST7789_WriteCmd(0xC6);   /* FRCTRL2 */
    ST7789_WriteData(0x0F);

    ST7789_WriteCmd(0xD0);   /* PWCTRL1 */
    ST7789_WriteData(0xA4);
    ST7789_WriteData(0xA1);

    ST7789_WriteCmd(0xE0);   /* PVGAMCTRL: 正极性gamma */
    ST7789_WriteData(0xD0);
    ST7789_WriteData(0x04);
    ST7789_WriteData(0x0D);
    ST7789_WriteData(0x11);
    ST7789_WriteData(0x13);
    ST7789_WriteData(0x2B);
    ST7789_WriteData(0x3F);
    ST7789_WriteData(0x54);
    ST7789_WriteData(0x4C);
    ST7789_WriteData(0x18);
    ST7789_WriteData(0x0D);
    ST7789_WriteData(0x0B);
    ST7789_WriteData(0x1F);
    ST7789_WriteData(0x23);

    ST7789_WriteCmd(0xE1);   /* NVGAMCTRL: 负极性gamma */
    ST7789_WriteData(0xD0);
    ST7789_WriteData(0x04);
    ST7789_WriteData(0x0C);
    ST7789_WriteData(0x11);
    ST7789_WriteData(0x13);
    ST7789_WriteData(0x2C);
    ST7789_WriteData(0x3F);
    ST7789_WriteData(0x44);
    ST7789_WriteData(0x51);
    ST7789_WriteData(0x2F);
    ST7789_WriteData(0x1F);
    ST7789_WriteData(0x1F);
    ST7789_WriteData(0x20);
    ST7789_WriteData(0x23);

    ST7789_WriteCmd(0x21);   /* INVON: 反转显示 */
    ST7789_WriteCmd(0x29);   /* DISPON: 开显示 */

    /* 清屏 */
    ST7789_FillScreen(COLOR_BLACK);

    /* 背光50% */
    ST7789_SetBacklight(50);
}

void ST7789_SetBacklight(uint8_t brightness) {
    if (brightness > 100) brightness = 100;
    uint32_t pulse = (uint32_t)(brightness * 10);  /* TIM3 ARR=999 */
    __HAL_TIM_SET_COMPARE(&htim3, TIM_CHANNEL_2, pulse);
}

void ST7789_SetWindow(uint16_t x, uint16_t y, uint16_t w, uint16_t h) {
    uint16_t x_end = x + w - 1;
    uint16_t y_end = y + h - 1;

    ST7789_WriteCmd(ST7789_CASET);
    ST7789_WriteData16(x);
    ST7789_WriteData16(x_end);

    ST7789_WriteCmd(ST7789_RASET);
    ST7789_WriteData16(y);
    ST7789_WriteData16(y_end);

    ST7789_WriteCmd(ST7789_RAMWR);
}

void ST7789_FillScreen(uint16_t color) {
    ST7789_FillRect(0, 0, ST7789_WIDTH, ST7789_HEIGHT, color);
}

void ST7789_FillRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    ST7789_SetWindow(x, y, w, h);

    ST7789_DC_DATA();
    ST7789_CS_LOW();

    uint32_t pixels = (uint32_t)w * h;
    uint8_t color_hi = (uint8_t)(color >> 8);
    uint8_t color_lo = (uint8_t)(color & 0xFF);

    for (uint32_t i = 0; i < pixels; i++) {
        HAL_SPI_Transmit(&hspi2, &color_hi, 1, HAL_MAX_DELAY);
        HAL_SPI_Transmit(&hspi2, &color_lo, 1, HAL_MAX_DELAY);
    }

    ST7789_CS_HIGH();
}

void ST7789_DrawPixel(uint16_t x, uint16_t y, uint16_t color) {
    if (x >= ST7789_WIDTH || y >= ST7789_HEIGHT) return;
    ST7789_SetWindow(x, y, 1, 1);
    ST7789_WriteData16(color);
}

void ST7789_DrawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t color) {
    int16_t dx = (int16_t)x1 - x0;
    int16_t dy = (int16_t)y1 - y0;
    int16_t steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
    if (steps == 0) { ST7789_DrawPixel(x0, y0, color); return; }

    float xi = (float)dx / steps;
    float yi = (float)dy / steps;
    float x = x0, y = y0;

    for (int16_t i = 0; i <= steps; i++) {
        ST7789_DrawPixel((uint16_t)x, (uint16_t)y, color);
        x += xi; y += yi;
    }
}

void ST7789_DrawRect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    ST7789_DrawLine(x, y, x + w, y, color);
    ST7789_DrawLine(x + w, y, x + w, y + h, color);
    ST7789_DrawLine(x + w, y + h, x, y + h, color);
    ST7789_DrawLine(x, y + h, x, y, color);
}

/* ── 8×16 ASCII字体 (精简, 可打印字符 0x20-0x7E) ── */
static const uint8_t font8x16[][16] = {
    [0]  = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, /* space */
    [1]  = {0x00,0x00,0x00,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x33,0x30,0x00,0x00,0x00}, /* ! */
    [16] = {0x00,0x10,0x60,0x80,0x00,0x80,0x60,0x10,0x00,0x08,0x06,0x01,0x00,0x01,0x06,0x08}, /* 0 */
    [17] = {0x00,0x00,0x20,0x20,0xF8,0x00,0x00,0x00,0x00,0x00,0x20,0x20,0x3F,0x20,0x20,0x00}, /* 1 */
    [18] = {0x00,0x70,0x88,0x88,0x90,0x60,0x00,0x00,0x00,0x30,0x28,0x24,0x22,0x21,0x30,0x00}, /* 2 */
    [19] = {0x00,0x70,0x88,0x88,0x88,0x70,0x00,0x00,0x00,0x18,0x20,0x20,0x20,0x11,0x0E,0x00}, /* 3 */
    [20] = {0x00,0x00,0x80,0x40,0x20,0xF8,0x00,0x00,0x00,0x07,0x04,0x04,0x04,0x3F,0x04,0x00}, /* 4 */
    [33] = {0x00,0x20,0x20,0x20,0xF8,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0F,0x10,0x20,0x00}, /* A */
    [34] = {0x00,0xF8,0x88,0x88,0x88,0x70,0x00,0x00,0x00,0x3F,0x20,0x20,0x20,0x11,0x0E,0x00}, /* B */
    [35] = {0x00,0xF8,0x80,0x80,0x80,0x80,0x00,0x00,0x00,0x07,0x08,0x10,0x10,0x10,0x08,0x00}, /* C */
    [48] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, /* . */
    [49] = {0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}, /* / */
    [50] = {0x00,0x80,0x80,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x01,0x00,0x00,0x00,0x00}, /* - */
};

void ST7789_DrawChar(uint16_t x, uint16_t y, char ch, uint16_t fg, uint16_t bg) {
    if (ch < ' ' || ch > '~') return;
    uint8_t idx = ch - ' ';

    ST7789_SetWindow(x, y, 8, 16);
    ST7789_DC_DATA();
    ST7789_CS_LOW();

    for (uint8_t row = 0; row < 16; row++) {
        uint8_t line = font8x16[idx][row];
        for (uint8_t col = 0; col < 8; col++) {
            uint16_t c = (line & (0x80 >> col)) ? fg : bg;
            uint8_t buf[2] = { (uint8_t)(c >> 8), (uint8_t)(c & 0xFF) };
            HAL_SPI_Transmit(&hspi2, buf, 2, HAL_MAX_DELAY);
        }
    }
    ST7789_CS_HIGH();
}

void ST7789_DrawString(uint16_t x, uint16_t y, const char *str, uint16_t fg, uint16_t bg) {
    while (*str) {
        if (x + 8 > ST7789_WIDTH) { x = 0; y += 16; }
        if (y + 16 > ST7789_HEIGHT) break;
        ST7789_DrawChar(x, y, *str++, fg, bg);
        x += 8;
    }
}

void ST7789_DrawInt(uint16_t x, uint16_t y, int32_t num, uint8_t len, uint16_t fg, uint16_t bg) {
    char buf[12];
    int pos = 0;
    uint8_t is_neg = 0;
    if (num < 0) { is_neg = 1; num = -num; }

    do { buf[pos++] = '0' + (num % 10); num /= 10; } while (num > 0);
    if (is_neg) buf[pos++] = '-';

    while (pos < len) buf[pos++] = ' ';
    for (int i = pos - 1; i >= 0; i--)
        ST7789_DrawChar(x + (len - 1 - i) * 8, y, buf[i], fg, bg);
}

void ST7789_DrawFloat(uint16_t x, uint16_t y, float num, uint8_t int_len, uint8_t dec_len, uint16_t fg, uint16_t bg) {
    uint8_t is_neg = 0;
    if (num < 0) { is_neg = 1; num = -num; }

    uint32_t int_part = (uint32_t)num;
    uint32_t dec_part = (uint32_t)((num - int_part) * powf(10, dec_len) + 0.5f);

    char buf[16];
    int pos = 0;
    for (uint8_t i = 0; i < dec_len; i++) { buf[pos++] = '0' + (dec_part % 10); dec_part /= 10; }
    buf[pos++] = '.';
    do { buf[pos++] = '0' + (int_part % 10); int_part /= 10; } while (int_part > 0);
    if (is_neg) buf[pos++] = '-';
    while (pos < (int)(int_len + dec_len + 1 + is_neg)) buf[pos++] = ' ';

    for (int i = pos - 1; i >= 0; i--)
        ST7789_DrawChar(x + (pos - 1 - i) * 8, y, buf[i], fg, bg);
}

void ST7789_DrawProgressBar(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint8_t progress, uint16_t bar_c, uint16_t bg_c) {
    ST7789_FillRect(x, y, w, h, bg_c);
    uint16_t bar_w = (uint16_t)((uint32_t)w * progress / 100);
    if (bar_w > 0) ST7789_FillRect(x, y, bar_w, h, bar_c);
}

void ST7789_DrawTitleBar(const char *title) {
    ST7789_FillRect(0, 0, ST7789_WIDTH, 20, COLOR_NAVY);
    ST7789_DrawString(4, 2, title, COLOR_WHITE, COLOR_NAVY);
    ST7789_DrawLine(0, 20, ST7789_WIDTH, 20, COLOR_CYAN);
}

void ST7789_DrawStatus(const char *mode, float freq, float vpp, const char *ftype) {
    /* 模式 */
    char buf[32];
    ST7789_DrawTitleBar(mode);

    /* 频率 */
    snprintf(buf, sizeof(buf), "F:%luHz", (uint32_t)freq);
    ST7789_DrawString(4, 25, buf, COLOR_CYAN, COLOR_BLACK);

    /* 峰峰值 */
    snprintf(buf, sizeof(buf), "Vpp:%.2fV", (double)vpp);
    ST7789_DrawString(4, 42, buf, COLOR_AMBER, COLOR_BLACK);

    /* 滤波类型 */
    snprintf(buf, sizeof(buf), "Type:%s", ftype);
    ST7789_DrawString(4, 59, buf, COLOR_GREEN, COLOR_BLACK);
}

void ST7789_DrawWaveform(const float *data, uint16_t len, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color) {
    ST7789_FillRect(x, y, w, h, COLOR_BLACK);
    ST7789_DrawRect(x, y, w, h, COLOR_DARK_GRAY);

    if (len < 2) return;
    uint16_t mid_y = y + h / 2;
    float scale = (float)(h - 4) / 2.0f;

    for (uint16_t i = 1; i < len; i++) {
        uint16_t x0 = x + (uint32_t)(i - 1) * w / len;
        uint16_t x1 = x + (uint32_t)i * w / len;
        int16_t y0_val = (int16_t)(mid_y - data[i - 1] * scale);
        int16_t y1_val = (int16_t)(mid_y - data[i] * scale);

        if (y0_val < (int16_t)y) y0_val = y;
        if (y0_val >= (int16_t)(y + h)) y0_val = y + h - 1;
        if (y1_val < (int16_t)y) y1_val = y;
        if (y1_val >= (int16_t)(y + h)) y1_val = y + h - 1;

        ST7789_DrawLine(x0, (uint16_t)y0_val, x1, (uint16_t)y1_val, color);
    }
}
