#include "history_fsm.h"
#include <string.h>

static size_t history_fsm_entry_count(history_fsm_t *h_fsm)
{
	return h_fsm->prediction_length_us / h_fsm->transition_resolution_us;
}

static size_t history_fsm_entry(history_fsm_t *h_fsm, sample_time_t time)
{
	size_t entries_count = history_fsm_entry_count(h_fsm);
	size_t i = time / h_fsm->transition_resolution_us;
	if (i > entries_count)
		i = entries_count - 1;
	return i;
}

history_fsm_t *history_fsm_create(sample_time_t prediction_length_us,
				  sample_time_t transition_resolution_us)
{
	history_fsm_t *h_fsm = malloc(sizeof(history_fsm_t));
	if (!h_fsm)
		return NULL;

	INIT_LIST_HEAD(&h_fsm->states);
	h_fsm->prediction_length_us = prediction_length_us;
	h_fsm->transition_resolution_us = transition_resolution_us;
	h_fsm->cur = NULL;

	return h_fsm;
}

int history_fsm_state_attach_metric(history_fsm_state_t *hf_state, const char *name, double value)
{
	history_fsm_state_metric_t *hfs_metric = malloc(sizeof(history_fsm_state_metric_t));
	if (!hfs_metric)
		return 1;

	INIT_LIST_HEAD(&hfs_metric->list);
	hfs_metric->name = strdup(name);
	hfs_metric->value = value;

	return 0;
}

static int history_fsm_state_transition_add(history_fsm_t *h_fsm, history_fsm_state_t *new_state)
{
	history_fsm_transition_t *hf_transition = malloc(sizeof(history_fsm_transition_t));
	if (!hf_transition)
		return 1;

	INIT_LIST_HEAD(&hf_transition->list);
	hf_transition->dst_state = new_state;
	hf_transition->cnt = calloc(history_fsm_entry_count(h_fsm), sizeof(uint32_t));

	return 0;
}

int history_fsm_state_add(history_fsm_t *h_fsm, fsm_state_t *user_fsm_state)
{
	history_fsm_state_t *pos;

	history_fsm_state_t *hf_state = malloc(sizeof(history_fsm_state_t));
	if (!hf_state)
		return 1;

	INIT_LIST_HEAD(&hf_state->list);
	INIT_LIST_HEAD(&hf_state->attachedMetrics);
	hf_state->user_fsm_state = user_fsm_state;

	list_for_each_entry(pos, &h_fsm->states, list) {
		history_fsm_state_transition_add(h_fsm, hf_state);
	}

	list_add(&hf_state->list, &h_fsm->states);

	if (!h_fsm->cur)
		h_fsm->cur = hf_state;

	return 0;
}

int history_fsm_state_changed(history_fsm_t *h_fsm, fsm_state_t *dst_fsm_state, sample_time_t time)
{
	history_fsm_transition_t *pos;

	if (!h_fsm->cur || h_fsm->cur->user_fsm_state == dst_fsm_state)
		return 0;

	/* look for the new state */
	list_for_each_entry(pos, &h_fsm->cur->transitions, list) {
		if (pos->dst_state->user_fsm_state == dst_fsm_state) {
			size_t i = history_fsm_entry(h_fsm, time);
			pos->cnt[i]++;
			return 0;
		}
	}

	return 1;
}

void history_fsm_reset_transitions(history_fsm_t *h_fsm)
{
	history_fsm_state_t *pos_s;
	history_fsm_transition_t *pos_t;
	int i;

	list_for_each_entry(pos_s, &h_fsm->states, list) {
		list_for_each_entry(pos_t, &pos_s->transitions, list) {
			for (i = 0; i < history_fsm_entry_count(h_fsm); i++)
				pos_t->cnt[i] = 0;
		}
	}
}

static void history_fsm_state_metric_delete(history_fsm_state_metric_t *hfs_metric)
{
	free(hfs_metric->name);
	free(hfs_metric);
}

static void history_fsm_state_delete(history_fsm_state_t *hf_state)
{
	history_fsm_state_metric_t *pos_m, *n_m;
	history_fsm_transition_t *pos_t, *n_t;

	list_for_each_entry_safe(pos_m, n_m, &hf_state->attachedMetrics, list) {
		list_del(&(pos_m->list));
		history_fsm_state_metric_delete(pos_m);
	}
	list_for_each_entry_safe(pos_t, n_t, &hf_state->transitions, list) {
		list_del(&(pos_t->list));
		free(pos_t);
	}
	free(hf_state);
}

void history_fsm_delete(history_fsm_t *h_fsm)
{
	history_fsm_state_t *pos, *n;

	list_for_each_entry_safe(pos, n, &h_fsm->states, list) {
		list_del(&(pos->list));
		history_fsm_state_delete(pos);
	}
	free(h_fsm);
}
