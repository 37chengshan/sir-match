#ifndef CONTROL_CAR_PID_H
#define CONTROL_CAR_PID_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float kp;
    float ki;
    float kd;
    float setpoint;
    float integral;
    float prev_error;
    float output_min;
    float output_max;
    float integral_min;
    float integral_max;
    unsigned char first_update;
} pid_t;

void pid_init(pid_t *pid,
              float kp,
              float ki,
              float kd,
              float output_min,
              float output_max,
              float integral_min,
              float integral_max);

void pid_reset(pid_t *pid);

void pid_set_target(pid_t *pid, float setpoint);

void pid_set_params(pid_t *pid, float kp, float ki, float kd);

void pid_set_limits(pid_t *pid,
                    float output_min,
                    float output_max,
                    float integral_min,
                    float integral_max);

float pid_update(pid_t *pid, float measurement, float dt_seconds);

#ifdef __cplusplus
}
#endif

#endif
