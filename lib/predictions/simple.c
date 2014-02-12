#include "prediction.h"
#include "../prediction_priv.h"

typedef struct {
	sample_time_t prediction_length;
} prediction_simple_t;

prediction_simple_t * prediction_simple(prediction_t* p)
{
	return (prediction_simple_t*) p->user;
}

int prediction_simple_check(prediction_t *p)
{
	return 0;
}

prediction_list_t *prediction_simple_exec(prediction_t *p)
{
	prediction_simple_t *simple = prediction_simple(p);
	prediction_metric_t *pos;

	prediction_list_t *pl = prediction_list_create();
	if (!pl)
		return NULL;

	/* all input metrics */
	list_for_each_entry(pos, prediction_metrics(p), list) {
		prediction_metric_result_t *r;

		if (metric_is_empty(pos->base))
			continue;

		r = prediction_metric_result_create(metric_name(pos->base), scoring_normal);

		/* read the history of the metric */
		r->hsize = metric_history_size(pos->base);
		r->history = calloc(r->hsize, sizeof(sample_t));
		r->hsize = metric_dump_history(pos->base, r->history, r->hsize);

		graph_add_point((graph_t *)r->high,
				0,
				r->history[r->hsize - 1].value);
		graph_add_point((graph_t *)r->high,
				simple->prediction_length,
				r->history[r->hsize - 1].value);

		graph_add_point((graph_t *)r->average,
				0,
				r->history[r->hsize - 1].value);
		graph_add_point((graph_t *)r->average,
				simple->prediction_length,
				r->history[r->hsize - 1].value);

		graph_add_point((graph_t *)r->low,
				0,
				r->history[r->hsize - 1].value);
		graph_add_point((graph_t *)r->low,
				simple->prediction_length,
				r->history[r->hsize - 1].value);

		prediction_list_append(pl, r);
	}

	return pl;
}

void prediction_simple_dtor(prediction_t *p)
{
	free(p->user);
}

prediction_t * prediction_simple_create(sample_time_t prediction_length)
{
	prediction_simple_t *simple = malloc(sizeof(prediction_simple_t));
	if (!simple)
		return NULL;

	simple->prediction_length = prediction_length;
	return prediction_create(prediction_simple_check,
				 prediction_simple_exec,
				 prediction_simple_dtor,
				 "pred_simple",
				 (void *)simple);
}
