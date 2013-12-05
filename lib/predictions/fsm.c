#include "prediction.h"
#include "../prediction_priv.h"
#include "predictions/fsm.h"

typedef struct {
	sample_time_t prediction_length;
	prediction_fsm_metric_from_state_t metric_state;
	const fsm_t *fsm;
} prediction_fsm_t;

prediction_fsm_t * prediction_fsm(prediction_t* p)
{
	return (prediction_fsm_t*) p->user;
}

int prediction_fsm_check(prediction_t *p)
{
	return 0;
}

prediction_list_t *prediction_fsm_exec(prediction_t *p)
{
	prediction_fsm_t *p_fsm = prediction_fsm(p);
	prediction_priv_t *p_priv = prediction_priv(p);
	prediction_metric_t *pos;

	prediction_list_t *pl = prediction_list_create();
	if (!pl)
		return NULL;

	/* all input metrics */
	list_for_each_entry(pos, &p_priv->metrics, list) {
		prediction_metric_result_t *r;
		r = prediction_metric_result_create(metric_name(pos->base));

		graph_add_point((graph_t *)r->high,
				0,
				metric_get_last(pos->base).value);
		graph_add_point((graph_t *)r->high,
				p_fsm->prediction_length,
				metric_get_last(pos->base).value);

		graph_add_point((graph_t *)r->average,
				0,
				metric_get_last(pos->base).value);
		graph_add_point((graph_t *)r->average,
				p_fsm->prediction_length,
				metric_get_last(pos->base).value);

		graph_add_point((graph_t *)r->low,
				0,
				metric_get_last(pos->base).value);
		graph_add_point((graph_t *)r->low,
				p_fsm->prediction_length,
				metric_get_last(pos->base).value);

		prediction_list_append(pl, r);
	}

	return pl;
}

void prediction_fsm_dtor(prediction_t *p)
{
	free(p->user);
}

prediction_t * prediction_fsm_create(const fsm_t *fsm,
				     prediction_fsm_metric_from_state_t metric_state,
				     sample_time_t prediction_length)
{
	prediction_fsm_t *p_fsm = malloc(sizeof(prediction_fsm_t));
	if (!p_fsm)
		return NULL;

	p_fsm->prediction_length = prediction_length;
	p_fsm->metric_state = metric_state;
	p_fsm->fsm = fsm;
	return prediction_create(prediction_fsm_check,
				 prediction_fsm_exec,
				 prediction_fsm_dtor,
				 (void *)p_fsm);
}
