#include "prediction.h"
#include "../../prediction_priv.h"
#include "predictions/pred_fsm.h"
#include "history_fsm.h"

#include <string.h>
#include <assert.h>

typedef struct {
	struct list_head list;
	char *name;
} prediction_fsm_output_metric_t;

typedef struct {
	sample_time_t prediction_length_us;
	sample_time_t transition_resolution_us;
	prediction_fsm_metric_from_state_t metric_state;
	fsm_t *fsm;

	history_fsm_t *hfsm;

	struct list_head output_metrics;
} prediction_fsm_t;


prediction_fsm_t * prediction_fsm(prediction_t* p)
{
	return (prediction_fsm_t*) p->user;
}

static int prediction_fsm_check(prediction_t *p)
{
	return 0;
}

prediction_list_t *prediction_fsm_exec(prediction_t *p)
{
	prediction_fsm_t *p_fsm = prediction_fsm(p);
	prediction_priv_t *p_priv = prediction_priv(p);
	prediction_metric_t *pos_m;
	sample_time_t last_change = 0;
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
	list_for_each_entry(pos_m, &p_priv->metrics, list) {
		hsize[i] = metric_history_size(pos_m->base);
		history[i] = calloc(hsize[i], sizeof(sample_t));
		metric_dump_history(pos_m->base, history[i], hsize[i]);
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

		/* init last change */
		if (last_change == 0)
			last_change = sample.time;

		/* update the user_fsm */
		new_state = fsm_update_state(p_fsm->fsm,
						metrics_name[min_time_idx],
						sample.value);

		/* update the history_fsm if needed */
		sample_time_t time = sample.time - last_change;

		if (history_fsm_state_changed(p_fsm->hfsm, new_state, time)) {
			/* add the missing state */
			history_fsm_state_add(p_fsm->hfsm, new_state);

			/* TODO: add the values of the output metrics */

			/* check everything is right */
			int ret = history_fsm_state_changed(p_fsm->hfsm, new_state, time);
			assert(ret == 0);

		}
		last_change = sample.time;
	}

	/* free all the metrics */
	i = 0;
	list_for_each_entry(pos_m, &p_priv->metrics, list) {
		free(history[i]);
		free(metrics_name[i]);
		i++;
	}
	free(history);
	free(metrics_name);
	free(cur_index);
	free(hsize);


	//history_fsm_state_trans_prob_density(p_fsm->fsm, )
	history_fsm_transitions_prob_density_to_csv(p_fsm->hfsm, "fsm_pred-throuput");

	/* start predicting */
		/* for all output metrics */
		/* output the different graphs */

	history_fsm_reset_transitions(p_fsm->hfsm);


	/* all input metrics */
	prediction_metric_t *pos;
	list_for_each_entry(pos, &p_priv->metrics, list) {
		prediction_metric_result_t *r;
		r = prediction_metric_result_create(metric_name(pos->base));

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
	}


	return pl;
}

static void prediction_fsm_dtor(prediction_t *p)
{
	prediction_fsm_t *p_fsm = prediction_fsm(p);
	history_fsm_delete(p_fsm->hfsm);
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
	p_fsm->hfsm = history_fsm_create(prediction_length_us, transition_resolution_us);

	return prediction_create(prediction_fsm_check,
				 prediction_fsm_exec,
				 prediction_fsm_dtor,
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
