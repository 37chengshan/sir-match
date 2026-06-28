/**
 * @file    main.c
 * @brief   电路模型探究装置 - 主程序
 * @note    STM32F407VET6 | 电赛G题 | 168MHz
 *          状态机: CALIBRATE → IDLE → SWEEP → AUTO_AMP → LEARN → REPLICATE
 */

#include "main.h"
#include "ad9833.h"
#include "adc_driver.h"
#include "dac_driver.h"
#include "st7789.h"
#include "pid.h"
#include "goertzel.h"
#include "filter_id.h"
#include "rlc_fit.h"
#include "waveform.h"

/* ══════════════════════════════════════════════════════════════
 * 全局变量
 * ══════════════════════════════════════════════════════════════ */
SystemState_t g_sys;

/* HAL句柄 */
SPI_HandleTypeDef  hspi1;
SPI_HandleTypeDef  hspi2;
ADC_HandleTypeDef  hadc1;
DAC_HandleTypeDef  hdac;
TIM_HandleTypeDef  htim2, htim3, htim6;
DMA_HandleTypeDef  hdma_adc1, hdma_dac1, hdma_spi2_tx;

/* 功能模块 */
static PID_t           g_pid;
static FreqResponse_t  g_freq_resp;
static FilterResult_t  g_filter_result;
static RLC_Params_t    g_rlc_params;
static WaveformReplicator_t g_replicator;

/* 按键状态 */
static uint32_t g_key_debounce_tick = 0;

/* ══════════════════════════════════════════════════════════════
 * 系统初始化
 * ══════════════════════════════════════════════════════════════ */
static void System_Init(void) {
    HAL_Init();
    SystemClock_Config();

    /* GPIO */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOC_CLK_ENABLE();

    /* SPI1 (AD9833) */
    __HAL_RCC_SPI1_CLK_ENABLE();
    hspi1.Instance = SPI1;
    hspi1.Init.Mode = SPI_MODE_MASTER;
    hspi1.Init.Direction = SPI_DIRECTION_2LINES;
    hspi1.Init.DataSize = SPI_DATASIZE_16BIT;
    hspi1.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi1.Init.CLKPhase = SPI_PHASE_2EDGE;
    hspi1.Init.NSS = SPI_NSS_SOFT;
    hspi1.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_4;
    hspi1.Init.FirstBit = SPI_FIRSTBIT_MSB;
    HAL_SPI_Init(&hspi1);

    /* SPI2 (ST7789) */
    __HAL_RCC_SPI2_CLK_ENABLE();
    hspi2.Instance = SPI2;
    hspi2.Init.Mode = SPI_MODE_MASTER;
    hspi2.Init.Direction = SPI_DIRECTION_2LINES;
    hspi2.Init.DataSize = SPI_DATASIZE_8BIT;
    hspi2.Init.CLKPolarity = SPI_POLARITY_LOW;
    hspi2.Init.CLKPhase = SPI_PHASE_1EDGE;
    hspi2.Init.NSS = SPI_NSS_SOFT;
    hspi2.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_2;  /* 42MHz */
    hspi2.Init.FirstBit = SPI_FIRSTBIT_MSB;
    HAL_SPI_Init(&hspi2);

    /* ADC1 */
    __HAL_RCC_ADC1_CLK_ENABLE();
    hadc1.Instance = ADC1;
    hadc1.Init.ClockPrescaler = ADC_CLOCK_SYNC_PCLK_DIV4;
    hadc1.Init.Resolution = ADC_RESOLUTION_12B;
    hadc1.Init.ScanConvMode = ENABLE;
    hadc1.Init.ContinuousConvMode = DISABLE;
    hadc1.Init.DiscontinuousConvMode = DISABLE;
    hadc1.Init.NbrOfConversion = 2;
    hadc1.Init.ExternalTrigConv = ADC_EXTERNALTRIGCONV_T2_CC2;
    hadc1.Init.ExternalTrigConvEdge = ADC_EXTERNALTRIGCONVEDGE_RISING;
    hadc1.Init.DMAContinuousRequests = ENABLE;
    hadc1.Init.EOCSelection = ADC_EOC_SEQ_CONV;
    HAL_ADC_Init(&hadc1);

    /* DAC1 */
    __HAL_RCC_DAC_CLK_ENABLE();
    hdac.Instance = DAC;
    HAL_DAC_Init(&hdac);

    /* TIM2 (ADC触发) */
    __HAL_RCC_TIM2_CLK_ENABLE();
    htim2.Instance = TIM2;
    htim2.Init.Prescaler = 0;
    htim2.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim2.Init.Period = 83;  /* 1MSPS */
    htim2.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_Base_Init(&htim2);

    /* TIM3 CH2 (背光PWM) */
    __HAL_RCC_TIM3_CLK_ENABLE();
    htim3.Instance = TIM3;
    htim3.Init.Prescaler = 83;
    htim3.Init.CounterMode = TIM_COUNTERMODE_UP;
    htim3.Init.Period = 999;
    htim3.Init.ClockDivision = TIM_CLOCKDIVISION_DIV1;
    HAL_TIM_PWM_Init(&htim3);

    /* TIM6 (DAC触发) */
    __HAL_RCC_TIM6_CLK_ENABLE();
    htim6.Instance = TIM6;
    htim6.Init.Prescaler = 0;
    htim6.Init.Period = 419;  /* 200kSPS */
    HAL_TIM_Base_Init(&htim6);

    /* DMA */
    __HAL_RCC_DMA2_CLK_ENABLE();

    /* 模块初始化 */
    AD9833_Init();
    ADC_Driver_Init();
    DAC_Driver_Init();
    ST7789_Init();

    /* PID初始化 */
    PID_Init(&g_pid, 0.5f, 100.0f, 0.001f, 0, 1.0f);

    /* 初始状态 */
    g_sys.mode = MODE_CALIBRATE;
    g_sys.freq_hz = DEFAULT_FREQ_HZ;
    g_sys.target_vpp = VPP_TARGET_DEFAULT;

    ST7789_DrawTitleBar("MATCH System v3.0");
    ST7789_DrawString(10, 30, "Calibrating...", COLOR_AMBER, COLOR_BLACK);
}

/* ══════════════════════════════════════════════════════════════
 * 按键扫描
 * ══════════════════════════════════════════════════════════════ */
static uint8_t Key_Read(uint16_t pin, GPIO_TypeDef *port) {
    if (HAL_GPIO_ReadPin(port, pin) == GPIO_PIN_RESET) return 1;
    return 0;
}

static uint8_t Key_Pressed(uint16_t pin, GPIO_TypeDef *port) {
    if (Key_Read(pin, port)) {
        uint32_t now = get_tick_ms();
        if (now - g_key_debounce_tick > KEY_DEBOUNCE_MS) {
            g_key_debounce_tick = now;
            return 1;
        }
    }
    return 0;
}

/* ══════════════════════════════════════════════════════════════
 * 自校准模式
 * ══════════════════════════════════════════════════════════════ */
static void Mode_Calibrate(void) {
    /* ADC自动零点校准 */
    ST7789_DrawString(10, 50, "ADC zero cal...", COLOR_CYAN, COLOR_BLACK);

    AD9833_Reset();
    HAL_Delay(10);

    /* 多次采样求零偏 */
    float offset_sum = 0;
    for (int i = 0; i < 64; i++) {
        uint16_t raw = ADC_ReadSingle(ADC_CHANNEL_0);
        offset_sum += ADC_TO_VOLTAGE(raw);
        HAL_Delay(1);
    }
    float offset = offset_sum / 64.0f;

    ST7789_DrawString(10, 70, "DAC cal...", COLOR_CYAN, COLOR_BLACK);

    /* DAC设中点 */
    DAC_SetDC(1.65f);

    /* AD9833恢复 */
    AD9833_Start();
    AD9833_SetFrequency(DEFAULT_FREQ_HZ);

    g_sys.calibration_done = 1;
    g_sys.mode = MODE_IDLE;

    ST7789_DrawStatus("IDLE", g_sys.freq_hz, g_sys.target_vpp, "--");
}

/* ══════════════════════════════════════════════════════════════
 * 自动幅度控制模式
 * ══════════════════════════════════════════════════════════════ */
static void Mode_AutoAmplitude(void) {
    /* 测量当前输出 */
    ADC_Measure_t meas;
    ADC_Measure(&meas, SAMPLES_PER_FREQ);
    g_sys.measured_vpp = meas.vpp;

    /* PID更新 */
    float output = PID_Compute(&g_pid, meas.vpp);

    /* 输出用于调节MCP41010数字电位器 */
    /* MCP41010: 0~255级, 阻值 0~10kΩ, 对应衰减比 */
    uint8_t pot_val = (uint8_t)(output * 255.0f);
    if (pot_val > 255) pot_val = 255;
    /* MCP41010_Write(pot_val);  -- 需要MCP41010驱动 */

    /* 显示状态 */
    char buf[32];
    snprintf(buf, sizeof(buf), "AUTO_AMP %lukHz", (uint32_t)(g_sys.freq_hz / 1000));
    ST7789_DrawStatus(buf, g_sys.freq_hz, meas.vpp,
                      FilterID_GetTypeName(g_sys.filter_type));
}

/* ══════════════════════════════════════════════════════════════
 * 扫频测量模式
 * ══════════════════════════════════════════════════════════════ */
static void Mode_Sweep(void) {
    FreqResp_Init(&g_freq_resp, 2.0f);  /* 参考输入2Vpp */

    ST7789_DrawTitleBar("SWEEPING...");
    int num_points = 50;  /* 对数间隔扫频 */

    for (int i = 0; i < num_points; i++) {
        /* 对数频率 */
        float ratio = (float)i / (num_points - 1);
        float freq = SWEEP_FREQ_LOW * powf(SWEEP_FREQ_HIGH / SWEEP_FREQ_LOW, ratio);

        /* 设置DDS */
        AD9833_SetFrequency((uint32_t)freq);
        HAL_Delay(20);

        /* 采集并计算 */
        float samples[SAMPLES_PER_FREQ];
        ADC_GetSamples(samples, SAMPLES_PER_FREQ, 0);

        float mag, phase;
        Goertzel_Compute(samples, SAMPLES_PER_FREQ, freq, ADC_SAMPLE_RATE, &mag, &phase);

        FreqResp_AddPoint(&g_freq_resp, freq, mag, phase);

        /* 进度条 */
        ST7789_DrawProgressBar(20, 110, 200, 10, (i + 1) * 100 / num_points,
                               COLOR_CYAN, COLOR_DARK_GRAY);
    }

    g_sys.sweep_ready = 1;

    /* 显示扫频曲线预览 */
    float freqs_arr[50], gains_arr[50], phases_arr[50];
    for (int i = 0; i < g_freq_resp.num_points && i < 50; i++) {
        freqs_arr[i] = g_freq_resp.points[i].freq;
        gains_arr[i] = g_freq_resp.points[i].gain_db;
        phases_arr[i] = g_freq_resp.points[i].phase_deg;
    }
    ST7789_DrawBode(freqs_arr, gains_arr, phases_arr,
                    g_freq_resp.num_points, 0, 20, 240, 180);
}

/* ══════════════════════════════════════════════════════════════
 * 自学习模式
 * ══════════════════════════════════════════════════════════════ */
static void Mode_Learn(void) {
    ST7789_DrawTitleBar("LEARNING...");

    /* 执行扫频 */
    Mode_Sweep();

    /* 滤波类型识别 */
    FilterID_Analyze(&g_freq_resp, &g_filter_result);
    g_sys.filter_type = g_filter_result.type;

    /* RLC参数拟合 */
    RLC_Fit(&g_filter_result, &g_freq_resp, &g_rlc_params);

    g_sys.cutoff_freq = g_rlc_params.fc;
    g_sys.quality_factor = g_rlc_params.Q;
    g_sys.R_val = g_rlc_params.R;
    g_sys.L_val = g_rlc_params.L;
    g_sys.C_val = g_rlc_params.C;
    g_sys.learn_done = 1;
    g_sys.mode = MODE_IDLE;

    /* 显示结果 */
    char buf[64];
    snprintf(buf, sizeof(buf), "LEARN DONE: %s",
             FilterID_GetTypeName(g_filter_result.type));
    ST7789_DrawTitleBar(buf);

    snprintf(buf, sizeof(buf), "f0:%.0fHz Q:%.2f",
             (double)g_rlc_params.fc, (double)g_rlc_params.Q);
    ST7789_DrawString(10, 25, buf, COLOR_CYAN, COLOR_BLACK);

    RLC_FormatString(&g_rlc_params, buf, sizeof(buf));
    ST7789_DrawString(10, 42, buf, COLOR_GREEN, COLOR_BLACK);
}

/* ══════════════════════════════════════════════════════════════
 * 波形复刻模式
 * ══════════════════════════════════════════════════════════════ */
static void Mode_Replicate(void) {
    ST7789_DrawTitleBar("REPLICATING...");

    /* 初始化复刻器 */
    Waveform_Init(&g_replicator);
    g_replicator.circuit_params = &g_rlc_params;
    g_replicator.freq_response = &g_freq_resp;

    /* 根据设置生成目标波形 */
    switch (g_sys.wave_type) {
    case WAVE_TYPE_SINE:
        Waveform_SetSine(&g_replicator, (float)g_sys.freq_hz, g_sys.target_vpp);
        break;
    case WAVE_TYPE_SQUARE:
        Waveform_SetSquare(&g_replicator, (float)g_sys.freq_hz,
                          g_sys.target_vpp, g_sys.wave_duty);
        break;
    case WAVE_TYPE_TRIANGLE:
        Waveform_SetTriangle(&g_replicator, (float)g_sys.freq_hz, g_sys.target_vpp);
        break;
    default:
        Waveform_SetSine(&g_replicator, (float)g_sys.freq_hz, g_sys.target_vpp);
        break;
    }

    /* 逆滤波补偿 */
    Waveform_Compensate(&g_replicator);

    /* 生成输出表 */
    Waveform_GenerateTable(&g_replicator);

    /* DAC输出 */
    Waveform_OutputViaDDS(&g_replicator);

    /* 评估质量 */
    float quality = Waveform_EvaluateQuality(&g_replicator);

    char buf[32];
    snprintf(buf, sizeof(buf), "REPLICATE OK Q:%.1f%%", (double)(100.0f - quality));
    ST7789_DrawStatus(buf, g_sys.freq_hz, g_sys.target_vpp,
                      FilterID_GetTypeName(g_sys.filter_type));
}

/* ══════════════════════════════════════════════════════════════
 * 主循环
 * ══════════════════════════════════════════════════════════════ */
int main(void) {
    System_Init();

    uint32_t last_tick = 0;

    while (1) {
        uint32_t now = get_tick_ms();

        /* 按键处理 */
        if (Key_Pressed(KEY_LEARN_PIN, KEY_LEARN_PORT)) {
            g_sys.mode = MODE_LEARN;
        }
        if (Key_Pressed(KEY_SET_PIN, KEY_SET_PORT)) {
            g_sys.freq_hz += FREQ_STEP_HZ;
            if (g_sys.freq_hz > SWEEP_FREQ_HIGH) g_sys.freq_hz = SWEEP_FREQ_LOW;
            g_sys.mode = MODE_AUTO_AMP;
        }
        if (Key_Pressed(KEY_START_PIN, KEY_START_PORT)) {
            if (g_sys.learn_done) {
                g_sys.mode = MODE_REPLICATE;
            } else {
                g_sys.mode = MODE_SWEEP;
            }
        }

        /* 状态机 */
        switch (g_sys.mode) {
        case MODE_CALIBRATE:
            Mode_Calibrate();
            break;
        case MODE_IDLE:
            /* 每250ms更新一次显示 */
            if (now - last_tick > 250) {
                last_tick = now;
                ST7789_DrawStatus("IDLE", g_sys.freq_hz, g_sys.target_vpp,
                                  g_sys.learn_done ?
                                  FilterID_GetTypeName(g_sys.filter_type) : "--");
            }
            break;
        case MODE_SWEEP:
            Mode_Sweep();
            g_sys.mode = MODE_IDLE;
            break;
        case MODE_AUTO_AMP:
            if (now - last_tick > 100) {
                last_tick = now;
                Mode_AutoAmplitude();
            }
            break;
        case MODE_LEARN:
            Mode_Learn();
            break;
        case MODE_REPLICATE:
            Mode_Replicate();
            g_sys.mode = MODE_IDLE;
            break;
        default:
            g_sys.mode = MODE_IDLE;
            break;
        }
    }
}

/* ══════════════════════════════════════════════════════════════
 * 系统时钟配置: 168MHz
 * HSE 8MHz → PLL×336 → /2 = 168MHz
 * APB1=42MHz, APB2=84MHz
 * ══════════════════════════════════════════════════════════════ */
void SystemClock_Config(void) {
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    __HAL_RCC_PWR_CLK_ENABLE();
    __HAL_PWR_VOLTAGESCALING_CONFIG(PWR_REGULATOR_VOLTAGE_SCALE1);

    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState = RCC_HSE_ON;
    RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLM = 8;
    RCC_OscInitStruct.PLL.PLLN = 336;
    RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
    RCC_OscInitStruct.PLL.PLLQ = 7;
    HAL_RCC_OscConfig(&RCC_OscInitStruct);

    RCC_ClkInitStruct.ClockType = RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_SYSCLK
                                | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
    RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;
    HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_5);
}

void Error_Handler(void) {
    __disable_irq();
    while (1) {
        HAL_GPIO_TogglePin(LED_PORT, LED_PIN);
        for (volatile uint32_t i = 0; i < 500000; i++);
    }
}

void delay_us(uint32_t us) {
    uint32_t ticks = us * (SYS_CLK_FREQ / 1000000) / 5;
    while (ticks--) __NOP();
}

uint32_t get_tick_ms(void) {
    return HAL_GetTick();
}

#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line) {
    (void)file; (void)line;
    Error_Handler();
}
#endif
