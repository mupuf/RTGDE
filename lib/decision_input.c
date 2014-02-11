#include "decision_priv.h"
#include <string.h>

decision_input_t *decision_input_create()
{
	decision_input_t *di = malloc(sizeof(decision_input_t));
	if (!di)
		return NULL;

	INIT_LIST_HEAD(&di->models);

	return di;
}

decision_input_model_t *decision_input_model_create(model_t *model)
{
	decision_input_model_t *dim = malloc(sizeof(decision_input_model_t));
	if (!dim)
		return NULL;

	INIT_LIST_HEAD(&dim->metrics);
	INIT_LIST_HEAD(&dim->list);
	dim->parent = NULL;
	dim->model = model;
	dim->score = 0;

	return dim;
}

decision_input_metric_t *decision_input_metric_create(const char *name,
					prediction_metric_result_t *prediction,
					graph_t *output)
{
	decision_input_metric_t *di_metric = malloc(sizeof(decision_input_metric_t));
	if (!di_metric)
		return NULL;

	INIT_LIST_HEAD(&di_metric->list);
	di_metric->parent = NULL;
	di_metric->name = strdup(name);
	di_metric->prediction = prediction;
	di_metric->output = output;
	di_metric->score = 0;

	return di_metric;
}

void decision_input_add_model(decision_input_t *di, decision_input_model_t *dim)
{
	if (dim->parent) {
		fprintf(stderr, "decision_input_add_model: "
			"This decision_input_model_t already has a parent\n");
		return;
	}
	dim->parent = di;
	list_add_tail(&dim->list, &di->models);
}

decision_input_model_t *decision_input_model_get_first(decision_input_t *di)
{
	if (list_empty(&di->models))
		return NULL;
	else
		return list_entry(di->models.next, decision_input_model_t, list);
}

decision_input_model_t *decision_input_model_get_next(decision_input_model_t *dim)
{
	struct list_head * next = dim->list.next;
	if (next == &dim->parent->models)
		return NULL;
	else
		return list_entry(dim->list.next, decision_input_model_t, list);
}

void decision_input_model_add_metric(decision_input_model_t *dim,
				     decision_input_metric_t *di_metric)
{
	if (!decision_input_metric_from_name(dim, di_metric->name)) {
		list_add_tail(&di_metric->list, &dim->metrics);
		di_metric->parent = dim;
	} else {
		fprintf(stderr, "decision_input_model_add_metric: "
			"cannot add another metric whose name is identical to "
			"one already-stored metric\n");
	}
}

decision_input_metric_t *decision_input_metric_from_name(decision_input_model_t *dim,
							 const char *name)
{
	decision_input_metric_t *pos;

	list_for_each_entry(pos, &dim->metrics, list) {
		if (strcmp(pos->name, name) ==0)
			return pos;
	}

	return NULL;
}

decision_input_metric_t *decision_input_metric_get_first(decision_input_model_t *dim)
{
	if (list_empty(&dim->metrics))
		return NULL;
	else
		return list_entry(dim->metrics.next, decision_input_metric_t, list);
}

decision_input_metric_t *decision_input_metric_get_next(decision_input_metric_t *di_metric)
{
	struct list_head * next = di_metric->list.next;
	if (next == &di_metric->parent->metrics)
		return NULL;
	else
		return list_entry(di_metric->list.next, decision_input_metric_t, list);
}

void decision_input_metric_delete(decision_input_metric_t *di_metric)
{
	if (!di_metric)
		return;

	free(di_metric->name);
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
	list_for_each_entry_safe(pos, n, &dim->metrics, list) {
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
