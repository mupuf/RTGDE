#include "model.h"
#include "../model_priv.h"
#include <assert.h>

typedef struct {
	/* Nothing here */
} model_dummy_t;

model_dummy_t * model_dummy(model_t* m)
{
	return (model_dummy_t*) model_priv(m)->user;
}

decision_input_model_t * model_dummy_exec(model_t *m, prediction_list_t *input)
{
	prediction_metric_result_t *metric;
	decision_input_metric_t *di_metric;
	decision_input_model_t *dim;

	dim = decision_input_model_create(m);
	if (!dim)
		 return NULL;

	do {
		metric = prediction_list_extract_head(input);
		if (!metric)
			continue;

		/* Real work goes here ! */
		graph_t * output = graph_create();

		const sample_t *s = graph_read_first(metric->high);
		while (s) {
			graph_add_point(output, s->time, s->value * 0.8);
			graph_add_point(output, s->time + 500000, s->value * 1.2);
			s = graph_read_next(metric->high, s);
		}

		di_metric = decision_input_metric_create(metric, output);
		decision_input_model_add_metric(dim, di_metric);
	} while (metric);

	return dim;
}

void model_dummy_delete(model_t *m)
{
	free(model_dummy(m));
}

model_t * model_dummy_create(const char *name)
{
	model_dummy_t *dummy = malloc(sizeof(model_dummy_t));
	if (!dummy)
		return NULL;

	return model_create(model_dummy_exec, model_dummy_delete, name, (void *)dummy);
}
