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

#ifndef DECISION_INPUT_H
#define DECISION_INPUT_H

#include "list.h"
#include "prediction.h"
#include <sys/time.h>

#ifdef __cplusplus
extern "C" {
#endif

/* decision input : Move that to a different file? */
typedef float score_t;

typedef struct {
	struct list_head models;
	struct timeval tv;
} decision_input_t;

typedef struct {
	struct list_head list;
	decision_input_t *parent;

	model_t *model;
	struct list_head metrics;
	score_t score;
} decision_input_model_t;

typedef struct {
	struct list_head list;
	decision_input_model_t *parent;
	char *name;

	prediction_metric_result_t *prediction;
	graph_t *output;
	score_t score;
} decision_input_metric_t;

decision_input_t * decision_input_create();
decision_input_model_t * decision_input_model_create(model_t *model);
decision_input_metric_t * decision_input_metric_create(const char *name,
						       prediction_metric_result_t *prediction,
						       graph_t *output);
void decision_input_add_model(decision_input_t *di, decision_input_model_t *dim);
decision_input_model_t *decision_input_model_get_first(decision_input_t *di);
decision_input_model_t *decision_input_model_get_next(decision_input_model_t *dim);
decision_input_model_t *decision_input_model_get_by_name(decision_input_t *di, const char *name);
void decision_input_model_add_metric(decision_input_model_t *dim,
				     decision_input_metric_t *di_metric);
decision_input_metric_t *decision_input_metric_from_name(decision_input_model_t *dim,
							 const char *name);
decision_input_metric_t *decision_input_metric_get_first(decision_input_model_t *dim);
decision_input_metric_t *decision_input_metric_get_next(decision_input_metric_t *di_metric);
void decision_input_model_delete(decision_input_model_t *dim);
void decision_input_delete(decision_input_t *di);

#ifdef __cplusplus
}
#endif

#endif // DECISION_INPUT_H
