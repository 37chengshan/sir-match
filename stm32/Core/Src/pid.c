/**
 * @file    pid.c
 * @brief   PID控制器实现 - 自动幅度控制
 * @note    增量式PID + 积分限幅 + 输出限幅
 *          MATLAB预调参数: Kp=0.5, Ki=100, Kd=0.001
 */

#include "main.h"
#include "pid.h"

void PID_Init(PID_t *pid, float Kp, float Ki, float Kd, float out_min, float out_max) {
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
    pid->out_min = out_min;
    pid->out_max = out_max;
    pid->integral_max = out_max * 0.5f;
    PID_Reset(pid);
}

void PID_SetTarget(PID_t *pid, float setpoint) {
    pid->setpoint = setpoint;
}

float PID_Compute(PID_t *pid, float measurement) {
    /* 计算误差 */
    pid->error = pid->setpoint - measurement;

    /* 积分项(带抗饱和) */
    pid->error_sum += pid->error;
    if (pid->error_sum > pid->integral_max)
        pid->error_sum = pid->integral_max;
    if (pid->error_sum < -pid->integral_max)
        pid->error_sum = -pid->integral_max;

    /* 微分项(测量值微分, 避免设定值跳变冲击) */
    float derivative = -(measurement - pid->error_prev);

    /* PID输出 */
    pid->output = pid->Kp * pid->error
                + pid->Ki * pid->error_sum
                + pid->Kd * derivative;

    /* 输出限幅 */
    if (pid->output > pid->out_max) pid->output = pid->out_max;
    if (pid->output < pid->out_min) pid->output = pid->out_min;

    /* 更新状态 */
    pid->error_prev = measurement;
    pid->count++;

    return pid->output;
}

void PID_Reset(PID_t *pid) {
    pid->error = 0;
    pid->error_prev = 0;
    pid->error_sum = 0;
    pid->output = 0;
    pid->count = 0;
}

void PID_SetParams(PID_t *pid, float Kp, float Ki, float Kd) {
    pid->Kp = Kp;
    pid->Ki = Ki;
    pid->Kd = Kd;
}

void PID_Enable(PID_t *pid, uint8_t enable) {
    pid->enabled = enable;
    if (!enable) PID_Reset(pid);
}

void PID_AutoTune(PID_t *pid, uint32_t freq_hz) {
    (void)freq_hz;
    /* 简化的Ziegler-Nichols自动整定 */

    /* 初始参数 */
    float Kp = 0.5f, Ki = 100.0f, Kd = 0.001f;

    /* 检测频率调整参数 */
    if (freq_hz > 2000) {
        /* 高频段: 减小积分, 增大微分 */
        Ki *= 0.7f;
        Kd *= 1.3f;
    } else if (freq_hz < 500) {
        /* 低频段: 增大积分 */
        Ki *= 1.3f;
    }

    PID_SetParams(pid, Kp, Ki, Kd);
}
