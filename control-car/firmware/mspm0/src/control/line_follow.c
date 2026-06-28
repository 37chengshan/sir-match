#include "line_follow.h"

static float default_weights_8[LINE_SENSOR_MAX_COUNT] = {
    -7.0f, -5.0f, -3.0f, -1.0f,
     1.0f,  3.0f,  5.0f,  7.0f
};

static float default_weights_5[LINE_SENSOR_MAX_COUNT] = {
    -4.0f, -2.0f, 0.0f, 2.0f, 4.0f, 0.0f, 0.0f, 0.0f
};

void line_sensor_model_init(line_sensor_model_t *model,
                            int sensor_count,
                            int active_level)
{
    int i;

    if (model == 0) {
        return;
    }

    if (sensor_count > LINE_SENSOR_MAX_COUNT) {
        sensor_count = LINE_SENSOR_MAX_COUNT;
    }
    if (sensor_count <= 0) {
        sensor_count = 5;
    }

    model->sensor_count = sensor_count;
    model->active_level = active_level ? 1 : 0;
    model->last_error = 0.0f;
    model->lost_count = 0u;

    for (i = 0; i < LINE_SENSOR_MAX_COUNT; ++i) {
        model->weights[i] = 0.0f;
    }

    if (sensor_count == 8) {
        for (i = 0; i < 8; ++i) {
            model->weights[i] = default_weights_8[i];
        }
    } else if (sensor_count == 5) {
        for (i = 0; i < 5; ++i) {
            model->weights[i] = default_weights_5[i];
        }
    } else {
        float center = (float)(sensor_count - 1) * 0.5f;
        for (i = 0; i < sensor_count; ++i) {
            model->weights[i] = ((float)i - center) * 2.0f;
        }
    }
}

float line_sensor_compute_error(line_sensor_model_t *model,
                                const unsigned char *digital_values)
{
    int i;
    int active_count = 0;
    float weighted_sum = 0.0f;

    if (model == 0 || digital_values == 0) {
        return 0.0f;
    }

    for (i = 0; i < model->sensor_count; ++i) {
        int active = (digital_values[i] ? 1 : 0) == model->active_level;
        if (active) {
            weighted_sum += model->weights[i];
            active_count += 1;
        }
    }

    if (active_count == 0) {
        model->lost_count += 1u;
        return model->last_error;
    }

    model->lost_count = 0u;
    model->last_error = weighted_sum / (float)active_count;
    return model->last_error;
}

unsigned int line_sensor_is_lost(const line_sensor_model_t *model)
{
    if (model == 0) {
        return 1u;
    }

    return model->lost_count > 0u;
}
