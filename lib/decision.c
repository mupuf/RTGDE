#include "decision.h"

decision_input_t *decision_input_create()
{
	decision_input_t *di = malloc(sizeof(decision_input_t));
	if (!di)
		return NULL;

	INIT_LIST_HEAD(&di->models);

	return di;
}

decision_input_model_t *decision_input_model_create()
{
	decision_input_model_t *dim = malloc(sizeof(decision_input_model_t));
	if (!dim)
		return NULL;

	INIT_LIST_HEAD(&dim->predictions);
	INIT_LIST_HEAD(&dim->list);

	return dim;
}

decision_input_metric_t *decision_input_metric_create(prediction_metric_result_t *prediction,
						      graph_t *output, int score)
{
	decision_input_metric_t *di_metric = malloc(sizeof(decision_input_metric_t));
	if (!di_metric)
		return NULL;

	INIT_LIST_HEAD(&di_metric->list);
	di_metric->prediction = prediction;
	di_metric->output = output;
	di_metric->score = score;

	return di_metric;
}

void decision_input_add_model(decision_input_t *di, decision_input_model_t *dim)
{
	list_add_tail(&dim->list, &di->models);
}

void decision_input_model_add_metric(decision_input_model_t *dim, decision_input_metric_t *di_metric)
{
	list_add_tail(&di_metric->list, &dim->predictions);
}

void decision_input_metric_delete(decision_input_metric_t *di_metric)
{
	if (!di_metric)
		return;

	prediction_metric_result_delete(di_metric->prediction);
	graph_delete(di_metric->output);
	free(di_metric);
}

void decision_input_model_delete(decision_input_model_t *dim)
{
	decision_input_metric_t *pos, *n;

	if (!dim)
		return;

	/* free the predications and decision list */
	list_for_each_entry_safe(pos, n, &dim->predictions, list) {
		list_del(&(pos->list));
		decision_input_metric_delete(pos);
	}
	free(dim);
}

void decision_input_delete(decision_input_t *di)
{
	decision_input_model_t *pos, *n;

	if (!di)
		return;

	/* free the predications and decision list */
	list_for_each_entry_safe(pos, n, &di->models, list) {
		list_del(&(pos->list));
		decision_input_model_delete(pos);
	}
	free(di);
}
