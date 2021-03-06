/*Copyright (c) 2014 Martin Peres
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "history_fsm.h"
#include <string.h>
#include <inttypes.h>
#include <assert.h>

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

static sample_time_t history_fsm_entry_to_time(history_fsm_t *h_fsm, size_t i)
{
	sample_time_t time = i * h_fsm->transition_resolution_us;
	return time;
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

int history_fsm_state_attach_metric(history_fsm_state_t *hf_state, const char *name, sample_value_t value)
{
	history_fsm_state_metric_t *hfs_metric = malloc(sizeof(history_fsm_state_metric_t));
	if (!hfs_metric)
		return 1;

	INIT_LIST_HEAD(&hfs_metric->list);
	hfs_metric->name = strdup(name);
	hfs_metric->value = value;

	return 0;
}

static int history_fsm_state_transition_add(history_fsm_t *h_fsm, history_fsm_state_t *state, history_fsm_state_t *new_state)
{
	int i;

	history_fsm_transition_t *hf_transition = malloc(sizeof(history_fsm_transition_t));
	if (!hf_transition)
		return 1;

	INIT_LIST_HEAD(&hf_transition->list);
	hf_transition->dst_state = new_state;
	hf_transition->cnt = calloc(history_fsm_entry_count(h_fsm), sizeof(uint32_t));
	for (i = 0; i < history_fsm_entry_count(h_fsm); i++)
		hf_transition->cnt[i] = 0;
	hf_transition->total_count = 0;
	list_add(&hf_transition->list, &state->transitions);

	return 0;
}

history_fsm_state_t *
history_fsm_state_add(history_fsm_t *h_fsm, fsm_state_t *user_fsm_state)
{
	history_fsm_state_t *pos;

	history_fsm_state_t *hf_state = malloc(sizeof(history_fsm_state_t));
	if (!hf_state)
		return NULL;

	INIT_LIST_HEAD(&hf_state->list);
	INIT_LIST_HEAD(&hf_state->attachedMetrics);
	INIT_LIST_HEAD(&hf_state->transitions);
	hf_state->user_fsm_state = user_fsm_state;

	list_for_each_entry(pos, &h_fsm->states, list) {
		history_fsm_state_transition_add(h_fsm, pos, hf_state);
		history_fsm_state_transition_add(h_fsm, hf_state, pos);
	}

	list_add(&hf_state->list, &h_fsm->states);

	if (!h_fsm->cur) {
		h_fsm->cur = hf_state;
		h_fsm->time_state_changed = (sample_time_t)-1;
	}

	return hf_state;
}

int history_fsm_state_changed(history_fsm_t *h_fsm, fsm_state_t *dst_fsm_state, sample_time_t time)
{
	history_fsm_transition_t *pos;

	if (!h_fsm->cur)
		return 1;

	if (h_fsm->cur->user_fsm_state == dst_fsm_state)
		return 0;

	if (time <= (h_fsm->time_state_changed + 1)) {
		assert(time > h_fsm->time_state_changed);
		return 0;
	}

	sample_time_t timer_diff = time - h_fsm->time_state_changed;

	/* look for the new state */
	list_for_each_entry(pos, &h_fsm->cur->transitions, list) {
		if (pos->dst_state->user_fsm_state == dst_fsm_state) {
			size_t i = history_fsm_entry(h_fsm, timer_diff);
			if (h_fsm->time_state_changed != (sample_time_t)-1) {
				pos->cnt[i]++;
				pos->total_count++;
			}
			h_fsm->cur = pos->dst_state;
			h_fsm->time_state_changed = time;
			return 0;
		}
	}

	return 1;
}

static history_fsm_state_t *
history_fsm_state_find(history_fsm_t *h_fsm, fsm_state_t *fsm_state)
{
	history_fsm_state_t *pos;

	/* look for the new state */
	list_for_each_entry(pos, &h_fsm->states, list) {
		if (pos->user_fsm_state == fsm_state) {
			return pos;
		}
	}
	return NULL;
}

static history_fsm_transition_t *
history_fsm_state_transition_find(history_fsm_t *h_fsm,
				  fsm_state_t *src_fsm_state,
				  fsm_state_t *dst_fsm_state)
{
	history_fsm_transition_t *pos;

	history_fsm_state_t *src = history_fsm_state_find(h_fsm, src_fsm_state);
	if (!src)
		return NULL;

	/* look for the new state */
	list_for_each_entry(pos, &src->transitions, list) {
		if (pos->dst_state->user_fsm_state == dst_fsm_state) {
			return pos;
		}
	}

	return NULL;
}

int history_fsm_state_trans_prob_density(history_fsm_t *h_fsm,
					 fsm_state_t *src_fsm_state,
					 fsm_state_t *dst_fsm_state,
					 FILE *stream)
{
	history_fsm_transition_t *trans ;
	int i;

	trans = history_fsm_state_transition_find(h_fsm, src_fsm_state, dst_fsm_state);

	if (!trans) {
		fprintf(stream, "Error: At least one of the state cannot be found\n");
		return 1;
	}

	fprintf(stream,
		"\"Time (µs, step = %"PRIu64")\", \"'%s'' -> '%s' probability density\"\n",
		h_fsm->transition_resolution_us,
		src_fsm_state->name, dst_fsm_state->name);

	/* loop pour afficher les valeurs ! */
	for (i = 0; i < history_fsm_entry_count(h_fsm); i++)
		fprintf(stream, "%"PRIu64", %i\n",
			history_fsm_entry_to_time(h_fsm, i),
			trans->cnt[i]);
	return 0;
}

static void csv_print_line(history_fsm_t *h_fsm, FILE *f, int i, const char *line)
{
	fprintf(f, "%"PRIu64"",
		    history_fsm_entry_to_time(h_fsm, i));
	fputs(line, f);
}

void history_fsm_transitions_prob_density_to_csv(history_fsm_t *h_fsm,
						 const char *basename,
						 int number)
{
	history_fsm_state_t *pos_s;
	history_fsm_transition_t *pos_t;
	int i;

	list_for_each_entry(pos_s, &h_fsm->states, list) {
		static char path[4096];
		snprintf(path, sizeof(path), "%s_%i_st_%s.csv",
			 basename, number, pos_s->user_fsm_state->name);

		FILE *f = fopen(path, "w");
		if (!f) {
			fprintf(stderr, "Err: Cannot open '%s'\n", path);
			continue;
		}

		/* HEADER */
		fprintf(f, "\"Time (µs, step = %"PRIu64")\"",
			h_fsm->transition_resolution_us);

		list_for_each_entry(pos_t, &pos_s->transitions, list) {
			fprintf(f, ", \"'%s' -> '%s' probability density\"",
				pos_s->user_fsm_state->name,
				pos_t->dst_state->user_fsm_state->name);
		}

		fprintf(f, "\n");

		/* VALUES */
		static char b1[1024], b2[1024];
		char *cur = b1, *last = NULL;
		size_t o = 0, last_printed = 0;
		for (i = 0; i < history_fsm_entry_count(h_fsm); i++) {
			o = 0;
			list_for_each_entry(pos_t, &pos_s->transitions, list) {
				o += snprintf(cur + o, 1024 - o, ", %f",
					((double)pos_t->cnt[i]) / pos_t->total_count);
			}
			snprintf(cur + o, 1024 - o, "\n");

			if (last) {
				if (strcmp(cur, last) != 0) {
					if (!last_printed)
						csv_print_line(h_fsm, f, i-1, last);
					csv_print_line(h_fsm, f, i, cur);
					last_printed = 1;
				} else
					last_printed = 0;
			} else
				csv_print_line(h_fsm, f, i, cur);

			last = cur;
			cur = (cur == b1 ? b2 : b1);
		}

		fclose(f);
	}
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
			pos_t->total_count = 0;
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
		free(pos_t->cnt);
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
