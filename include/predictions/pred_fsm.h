#ifndef PREDICTION_FSM_H
#define PREDICTION_FSM_H

#include "../prediction.h"
#include "../fsm.h"

typedef int (*prediction_fsm_metric_from_state_t)(fsm_state_t *state,
						  const char *metric_name,
						  sample_value_t *value);

prediction_t *prediction_fsm_create(fsm_t *fsm,
				    prediction_fsm_metric_from_state_t metric_state,
				    sample_time_t prediction_length,
				    sample_time_t transition_resolution_us);

int prediction_fsm_add_output_metric(prediction_t *pred_fsm, const char *metric_name);
void prediction_fsm_dump_probability_density(prediction_t *pred_fsm,
					    const char *filename_pattern);

#endif
