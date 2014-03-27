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

#include "model.h"
#include "../model_priv.h"
#include <assert.h>

typedef struct {
	/* Nothing here */
} model_dummy_t;

model_dummy_t * model_dummy(model_t* m)
{
	return (model_dummy_t*) model_user(m);
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
			graph_add_point(output, s->time + 1000000, s->value * 1.2);
			s = graph_read_next(metric->high, s);
		}

		di_metric = decision_input_metric_create(metric->name,
							 metric, output);
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
