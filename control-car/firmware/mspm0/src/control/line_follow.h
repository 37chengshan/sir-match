#ifndef CONTROL_CAR_LINE_FOLLOW_H
#define CONTROL_CAR_LINE_FOLLOW_H

#ifdef __cplusplus
extern "C" {
#endif

#define LINE_SENSOR_MAX_COUNT 8

typedef struct {
    int sensor_count;
    int active_level;
    float weights[LINE_SENSOR_MAX_COUNT];
    float last_error;
    unsigned int lost_count;
} line_sensor_model_t;

void line_sensor_model_init(line_sensor_model_t *model,
                            int sensor_count,
                            int active_level);

float line_sensor_compute_error(line_sensor_model_t *model,
                                const unsigned char *digital_values);

unsigned int line_sensor_is_lost(const line_sensor_model_t *model);

#ifdef __cplusplus
}
#endif

#endif
