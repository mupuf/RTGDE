#ifndef PREDICTION_CONSTRAINT_H
#define PREDICTION_CONSTRAINT_H

#include "../prediction.h"

#ifdef __cplusplus
extern "C" {
#endif

prediction_t * prediction_constraint_create(const char *constraint_name,
					    sample_time_t prediction_length,
					    sample_value_t low,
					    sample_value_t avg,
					    sample_value_t high,
					    score_simple_style_t scoring_style);

#ifdef __cplusplus
}
#endif

#endif
