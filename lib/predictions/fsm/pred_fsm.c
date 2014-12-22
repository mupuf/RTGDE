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

#include "prediction.h"
#include "../../prediction_priv.h"
#include "predictions/pred_fsm.h"
#include "history_fsm.h"

#include <string.h>
#include <assert.h>

#define PREDICTION_NAME "pred_fsm"

typedef struct {
	struct list_head list;
	char *name;
	sample_value_t value;
} prediction_fsm_output_metric_t;

typedef struct {
	sample_time_t prediction_length_us;
	sample_time_t transition_resolution_us;
	prediction_fsm_metric_from_state_t metric_state;
	fsm_t *fsm;
	char *prob_density_filename;

	history_fsm_t *hfsm;

	struct list_head output_metrics;
} prediction_fsm_t;


prediction_fsm_t * prediction_fsm(prediction_t* p)
{
	if (strcmp(prediction_name(p), PREDICTION_NAME) == 0)
		return (prediction_fsm_t*) p->user;
	else {
		fprintf(stderr,
			"Tried to transtype prediction '%s' into '%s'. Abort.\n",
			prediction_name(p), PREDICTION_NAME);
		return NULL;
	}
}

static int prediction_fsm_check(prediction_t *p)
{
	return 0;
}

prediction_list_t *prediction_fsm_exec(prediction_t *p)
{
	prediction_fsm_t *p_fsm = prediction_fsm(p);
	prediction_fsm_output_metric_t *pos_mo;
	prediction_metric_t *pos_m;
	int i = 0;

	prediction_list_t *pl = prediction_list_create();
	if (!pl)
		return NULL;

	/* read the history of all metrics */
	uint32_t metrics_count = prediction_metrics_count(p);
	sample_t **history = calloc(metrics_count, sizeof(sample_t *));
	char **metrics_name = calloc(metrics_count, sizeof(char *));
	size_t *cur_index = calloc(metrics_count, sizeof(size_t));
	history_size_t *hsize = calloc(metrics_count, sizeof(history_size_t));
	list_for_each_entry(pos_m, prediction_metrics(p), list) {
		hsize[i] = metric_history_size(pos_m->base);
		history[i] = calloc(hsize[i], sizeof(sample_t));
		hsize[i] = metric_dump_history(pos_m->base, history[i], hsize[i]);
		cur_index[i] = 0;
		metrics_name[i] = strdup(metric_name(pos_m->base));
		i++;
	}

	/* train the model */
	while (1) {
		fsm_state_t *new_state;

		/* for all metrics, select the one with the lowest timestamp */
		size_t min_time_idx = 0;
		sample_time_t min_time = (sample_time_t)-1;
		for (i = 0; i < metrics_count; i++) {
			sample_time_t mtime = history[i][cur_index[i]].time;
			if (mtime < min_time && hsize[i] > cur_index[i]) {
				min_time = mtime;
				min_time_idx = i;
			}
		}

		/* exit if we didn't find one ! */
		if (min_time == (sample_time_t)-1)
			break;

		/* we have selected a value, consume it */
		sample_t sample = history[min_time_idx][cur_index[min_time_idx]];
		cur_index[min_time_idx]++;

		/* update the user_fsm */
		new_state = fsm_update_state(p_fsm->fsm,
						metrics_name[min_time_idx],
						sample.value);

		if (history_fsm_state_changed(p_fsm->hfsm, new_state, sample.time)) {
			history_fsm_state_t *fsm_state;

			/* add the missing state */
			fsm_state = history_fsm_state_add(p_fsm->hfsm, new_state);

			/* add the values of the output metrics */
			list_for_each_entry(pos_mo, &p_fsm->output_metrics, list) {
				if (!p_fsm->metric_state(new_state, pos_mo->name, &pos_mo->value)) {
					fprintf(stderr, "pred_fsm: cannot fetch the the value "
						"associated with metric '%s' for the state '%s'\n",
						pos_mo->name, new_state->name);
				}

				history_fsm_state_attach_metric(fsm_state,
								pos_mo->name,
								pos_mo->value);
			}

			/* check everything is right */
			int ret = history_fsm_state_changed(p_fsm->hfsm, new_state, sample.time);
			assert(ret == 0);

		}
	}


	//history_fsm_state_trans_prob_density(p_fsm->fsm, )
	if (p_fsm->prob_density_filename) {
		history_fsm_transitions_prob_density_to_csv(p_fsm->hfsm,
							    p_fsm->prob_density_filename,
							    prediction_metrics_count(p));
	}

	/* start predicting */
		/* for all output metrics */

		/* output the different graphs */

	history_fsm_reset_transitions(p_fsm->hfsm);


	/* generate the output */
	prediction_metric_t *pos;
	i = 0;
	list_for_each_entry(pos, prediction_metrics(p), list) {
		prediction_metric_result_t *r;

		if (metric_is_empty(pos->base))
			continue;

		r = prediction_metric_result_create(metric_name(pos->base),
						    metric_unit(pos->base),
						    scoring_normal);
		/* read the history of the metric */
		r->hsize = metric_history_size(pos->base);
		r->history = calloc(r->hsize, sizeof(sample_t));
		r->hsize = metric_dump_history(pos->base, r->history, r->hsize);
		r->history_start = 0;
		r->history_stop = r->hsize;

		graph_add_point((graph_t *)r->high,
				0,
				metric_get_last(pos->base).value);
		graph_add_point((graph_t *)r->high,
				p_fsm->prediction_length_us,
				metric_get_last(pos->base).value);

		graph_add_point((graph_t *)r->average,
				0,
				metric_get_last(pos->base).value);
		graph_add_point((graph_t *)r->average,
				p_fsm->prediction_length_us,
				metric_get_last(pos->base).value);

		graph_add_point((graph_t *)r->low,
				0,
				metric_get_last(pos->base).value);
		graph_add_point((graph_t *)r->low,
				p_fsm->prediction_length_us,
				metric_get_last(pos->base).value);

		prediction_list_append(pl, r);

		free(history[i]);
		free(metrics_name[i]);

		i++;
	}

	/* free all the metrics */
	free(history);
	free(metrics_name);
	free(cur_index);
	free(hsize);

	return pl;
}

static void prediction_fsm_dtor(prediction_t *p)
{
	prediction_fsm_output_metric_t *pos, *tmp;

	prediction_fsm_t *p_fsm = prediction_fsm(p);
	history_fsm_delete(p_fsm->hfsm);
	if (p_fsm->prob_density_filename)
		free(p_fsm->prob_density_filename);

	list_for_each_entry_safe(pos, tmp, &p_fsm->output_metrics, list) {
		list_del(&(pos->list));
		free(pos->name);
		free(pos);
	}

	free(p->user);
}

prediction_t * prediction_fsm_create(fsm_t *fsm,
				     prediction_fsm_metric_from_state_t metric_state,
				     sample_time_t prediction_length_us,
				     sample_time_t transition_resolution_us)
{
	prediction_fsm_t *p_fsm = (prediction_fsm_t *) malloc(sizeof(prediction_fsm_t));
	if (!p_fsm)
		return NULL;

	p_fsm->prediction_length_us = prediction_length_us;
	p_fsm->transition_resolution_us = transition_resolution_us;
	p_fsm->metric_state = metric_state;
	p_fsm->fsm = fsm;
	p_fsm->prob_density_filename = NULL;
	p_fsm->hfsm = history_fsm_create(prediction_length_us, transition_resolution_us);
	INIT_LIST_HEAD(&p_fsm->output_metrics);

	return prediction_create(prediction_fsm_check,
				 prediction_fsm_exec,
				 prediction_fsm_dtor,
				 "pred_fsm",
				 (void *)p_fsm);
}

int prediction_fsm_add_output_metric(prediction_t *pred_fsm, const char *metric_name)
{
	prediction_fsm_output_metric_t *fom = malloc(sizeof(prediction_fsm_output_metric_t));
	if (!fom)
		return 1;

	INIT_LIST_HEAD(&fom->list);
	fom->name = strdup(metric_name);
	list_add(&fom->list, &prediction_fsm(pred_fsm)->output_metrics);

	return 0;
}

void prediction_fsm_dump_probability_density(prediction_t *pred_fsm,
					    const char *base_filename)
{
	prediction_fsm_t *p_fsm = prediction_fsm(pred_fsm);
	p_fsm->prob_density_filename = strdup(base_filename);
}
