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

#ifndef PREDICTION_H
#define PREDICTION_H

#include "metric.h"
#include "graph.h"
#include "list.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	void *user;
} prediction_t;

typedef enum
{
	pmr_usage_prediction = 0,
	pmr_usage_constraint
} pmr_usage_hint_t;

typedef enum {
	scoring_normal = 0,
	scoring_inverted = 1
} score_simple_style_t;

typedef struct {
	struct list_head list;

	const char *name;
	const char *unit;
	score_simple_style_t scoring_style;
	pmr_usage_hint_t usage_hint;

	/* input */
	metric_t *metric;
	history_size_t hsize;
	sample_t *history;
	history_size_t history_start;
	history_size_t history_stop;

	/* prediction output */
	const graph_t *high;
	const graph_t *average;
	const graph_t *low;
} prediction_metric_result_t;


typedef struct list_head prediction_list_t;

const char *pmr_usage_hint_to_str(pmr_usage_hint_t hint);

prediction_list_t * prediction_list_create();
prediction_metric_result_t *prediction_metric_result_create(const char *name, const char *unit, score_simple_style_t scoring_style);
prediction_metric_result_t *prediction_metric_result_copy(prediction_metric_result_t *pmr);
int prediction_list_append_list_copy(prediction_list_t *po, const prediction_list_t *npl);
prediction_metric_result_t * prediction_list_find(prediction_list_t *input, const char *metric_name);
prediction_metric_result_t * prediction_list_get_first(prediction_list_t *input);
prediction_metric_result_t * prediction_list_get_next(prediction_list_t *input, prediction_metric_result_t *pmr);
prediction_metric_result_t * prediction_list_extract_by_name(prediction_list_t *input, const char *metric_name);
prediction_metric_result_t * prediction_list_extract_head(prediction_list_t *input);
void prediction_metric_result_delete(prediction_metric_result_t *pmr);
void prediction_list_delete(prediction_list_t *po);

typedef int (*prediction_check_t)(prediction_t *p);
typedef prediction_list_t *(*prediction_exec_t)(prediction_t *p);
typedef void (*prediction_delete_t)(prediction_t *p);

typedef struct {
	metric_t *base;

	/* private declarations */
	struct list_head list;
} prediction_metric_t;

prediction_t * prediction_create(prediction_check_t check,
				 prediction_exec_t exec,
				 prediction_delete_t dtor,
				 const char *name,
				 void *user);
const char *prediction_name(prediction_t* p);
void prediction_list_append(prediction_list_t *pl, prediction_metric_result_t *pmr);
uint32_t prediction_metrics_count(prediction_t* p);
struct list_head *prediction_metrics(prediction_t *p);
int prediction_attach_metric(prediction_t *p, metric_t *m);
metric_t *prediction_find_metric(prediction_t *p, const char *name);
prediction_list_t *prediction_exec(prediction_t *p);
void prediction_delete(prediction_t *p);

#ifdef __cplusplus
}
#endif

#endif // PREDICTION_H
