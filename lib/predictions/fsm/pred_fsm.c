#include "prediction.h"
#include "../../prediction_priv.h"
#include "predictions/pred_fsm.h"

#include <string.h>

typedef struct {
	struct list_head list;
	char *name;
} prediction_fsm_output_metric_t;

typedef struct {
	sample_time_t prediction_length_us;
	sample_time_t transition_resolution_us;
	prediction_fsm_metric_from_state_t metric_state;
	const fsm_t *fsm;

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
	prediction_metric_t *pos_m, *n_m;

	prediction_list_t *pl = prediction_list_create();
	if (!pl)
		return NULL;

	/* train the model */
	list_for_each_entry_safe(pos_m, n_m, &p_priv->metrics, list) {
		/* for all metrics, select the one with the lowest timestamp */
		/* update the user_fsm */
		/* update the history_fsm if needed */
	}


	/* start predicting */
		/* for all output metrics */
		/* output the different graphs */


	return pl;
}

static void prediction_fsm_dtor(prediction_t *p)
{
	free(p->user);
}

prediction_t * prediction_fsm_create(const fsm_t *fsm,
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
