#include "pid.h"

static float clamp_float(float value, float min_value, float max_value)
{
    if (value < min_value) {
        return min_value;
    }
    if (value > max_value) {
        return max_value;
    }
    return value;
}

void pid_init(pid_t *pid,
              float kp,
              float ki,
              float kd,
              float output_min,
              float output_max,
              float integral_min,
              float integral_max)
{
    if (pid == 0) {
        return;
    }

    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
    pid->setpoint = 0.0f;
    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
    pid->output_min = output_min;
    pid->output_max = output_max;
    pid->integral_min = integral_min;
    pid->integral_max = integral_max;
    pid->first_update = 1u;
}

void pid_reset(pid_t *pid)
{
    if (pid == 0) {
        return;
    }

    pid->integral = 0.0f;
    pid->prev_error = 0.0f;
    pid->first_update = 1u;
}

void pid_set_target(pid_t *pid, float setpoint)
{
    if (pid == 0) {
        return;
    }

    pid->setpoint = setpoint;
}

void pid_set_params(pid_t *pid, float kp, float ki, float kd)
{
    if (pid == 0) {
        return;
    }

    pid->kp = kp;
    pid->ki = ki;
    pid->kd = kd;
}

void pid_set_limits(pid_t *pid,
                    float output_min,
                    float output_max,
                    float integral_min,
                    float integral_max)
{
    if (pid == 0) {
        return;
    }

    pid->output_min = output_min;
    pid->output_max = output_max;
    pid->integral_min = integral_min;
    pid->integral_max = integral_max;
}

float pid_update(pid_t *pid, float measurement, float dt_seconds)
{
    float error;
    float derivative;
    float output;

    if (pid == 0 || dt_seconds <= 0.0f) {
        return 0.0f;
    }

    error = pid->setpoint - measurement;

    pid->integral += error * dt_seconds;
    pid->integral = clamp_float(pid->integral, pid->integral_min, pid->integral_max);

    if (pid->first_update) {
        derivative = 0.0f;
        pid->first_update = 0u;
    } else {
        derivative = (error - pid->prev_error) / dt_seconds;
    }

    pid->prev_error = error;

    output = pid->kp * error + pid->ki * pid->integral + pid->kd * derivative;
    output = clamp_float(output, pid->output_min, pid->output_max);

    return output;
}
