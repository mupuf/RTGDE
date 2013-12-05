#ifndef PREDICTION_FSM_H
#define PREDICTION_FSM_H

#include "../prediction.h"
#include "../fsm.h"

typedef sample_value_t (*prediction_fsm_metric_from_state_t)(const fsm_t *fsm, const char *metric_name);

prediction_t *prediction_fsm_create(const fsm_t *fsm,
				    prediction_fsm_metric_from_state_t metric_state,
				    sample_time_t prediction_length);


#endif
