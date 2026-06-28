/**
 * @file    pid.h
 * @brief   PID控制器 - 自动幅度控制
 * @note    增量式PID + 积分限幅 + 输出限幅
 *          用于调节AD9833输出幅度使被测电路输出稳定在目标值
 *
 * 控制环路:
 *   目标Vpp -> [PID] -> AD9833幅度字 -> 被测电路 -> ADC测量Vpp -> 反馈
 *
 * 由于AD9833本身无幅度控制引脚, 此处通过
 * 数字电位器或PWM+RC+运放衰减网络来调节幅度
 * 如果硬件使用DAC输出+运放方案, 则控制DAC输出值
 */

#ifndef __PID_H
#define __PID_H

#include "main.h"

/* ======================== 数据结构 ======================== */

typedef struct {
    /* PID参数 */
    float Kp;           /* 比例系数 */
    float Ki;           /* 积分系数 */
    float Kd;           /* 微分系数 */

    /* 状态变量 */
    float error;        /* 当前误差 */
    float error_prev;   /* 上次误差 */
    float error_sum;    /* 误差累积(积分项) */
    float output;       /* 控制输出 */

    /* 限幅参数 */
    float out_min;      /* 输出下限 */
    float out_max;      /* 输出上限 */
    float integral_max; /* 积分项上限(抗饱和) */

    /* 目标值 */
    float setpoint;     /* 目标值 */

    /* 运行状态 */
    uint8_t enabled;    /* PID使能标志 */
    uint32_t count;     /* 运行次数计数 */
} PID_t;

/* ======================== 函数声明 ======================== */

/**
 * @brief  初始化PID控制器
 * @param  pid: PID结构体指针
 * @param  Kp: 比例系数
 * @param  Ki: 积分系数
 * @param  Kd: 微分系数
 * @param  out_min: 输出下限
 * @param  out_max: 输出上限
 */
void PID_Init(PID_t *pid, float Kp, float Ki, float Kd,
              float out_min, float out_max);

/**
 * @brief  设置PID目标值
 * @param  pid: PID结构体指针
 * @param  setpoint: 目标值
 */
void PID_SetTarget(PID_t *pid, float setpoint);

/**
 * @brief  运行一步PID计算
 * @param  pid: PID结构体指针
 * @param  measurement: 当前测量值
 * @return 控制输出值
 */
float PID_Compute(PID_t *pid, float measurement);

/**
 * @brief  重置PID状态(保留参数)
 * @param  pid: PID结构体指针
 */
void PID_Reset(PID_t *pid);

/**
 * @brief  更新PID参数(运行时调参)
 * @param  pid: PID结构体指针
 * @param  Kp: 新比例系数
 * @param  Ki: 新积分系数
 * @param  Kd: 新微分系数
 */
void PID_SetParams(PID_t *pid, float Kp, float Ki, float Kd);

/**
 * @brief  使能/禁用PID
 * @param  pid: PID结构体指针
 * @param  enable: 1=使能, 0=禁用
 */
void PID_Enable(PID_t *pid, uint8_t enable);

/**
 * @brief  自动整定PID参数(基于Ziegler-Nichols简化版)
 * @param  pid: PID结构体指针
 * @param  freq_hz: 当前工作频率(不同频率可能需要不同参数)
 */
void PID_AutoTune(PID_t *pid, uint32_t freq_hz);

#endif /* __PID_H */
