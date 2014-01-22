#ifndef HISTORY_FSM_H
#define HISTORY_FSM_H

#include <stdint.h>

#include "list.h"
#include "fsm.h"
#include "sample.h"

typedef struct {
	struct list_head list;
	char *name;
	double value;
} history_fsm_state_metric_t;

typedef struct {
	struct list_head list;
	fsm_state_t *user_fsm_state;

	struct list_head attachedMetrics;
	struct list_head transitions;
} history_fsm_state_t;

typedef struct {
	struct list_head list;
	history_fsm_state_t *dst_state;
	uint32_t *cnt;
	uint64_t total_count;
} history_fsm_transition_t;

typedef struct {
	struct list_head states;

	sample_time_t prediction_length_us;
	sample_time_t transition_resolution_us;

	history_fsm_state_t *cur;
	sample_time_t time_state_changed;
} history_fsm_t;

history_fsm_t *history_fsm_create(sample_time_t prediction_length_us,
				  sample_time_t transition_resolution_us);
history_fsm_state_t *history_fsm_state_add(history_fsm_t *h_fsm, fsm_state_t *user_fsm_state);
int history_fsm_state_attach_metric(history_fsm_state_t *hf_state, const char *name, sample_value_t value);
int history_fsm_state_changed(history_fsm_t *h_fsm, fsm_state_t *dst_fsm_state, sample_time_t time);
int history_fsm_state_trans_prob_density(history_fsm_t *h_fsm, fsm_state_t *src_fsm_state,
					 fsm_state_t *dst_fsm_state, FILE *stream);
void history_fsm_transitions_prob_density_to_csv(history_fsm_t *h_fsm, const char *basename, int number);
void history_fsm_reset_transitions(history_fsm_t *h_fsm);
void history_fsm_delete(history_fsm_t *h_fsm);

#endif // HISTORY_FSM_H
