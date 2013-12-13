#ifndef PREDICTION_SIMPLE_H
#define PREDICTION_SIMPLE_H

#include "../prediction.h"

#ifdef __cplusplus
extern "C" {
#endif

prediction_t * prediction_simple_create(sample_time_t prediction_length);

#ifdef __cplusplus
}
#endif

#endif
