#ifndef PREDICTION_H
#define PREDICTION_H

#include "metric.h"
#include "graph.h"
#include "list.h"

typedef struct {
	void *user;
} prediction_t;

typedef struct {
	struct list_head list;

	const char *name;

	/* prediction output */
	const graph_t *high;
	const graph_t *average;
	const graph_t *low;
} prediction_metric_result_t;

typedef struct list_head prediction_list_t;

prediction_list_t * prediction_list_create();
int prediction_list_append_list_copy(prediction_list_t *po, const prediction_list_t *npl);
prediction_metric_result_t * prediction_list_find(prediction_list_t *input, const char *metric_name);
prediction_metric_result_t * prediction_list_extract_by_name(prediction_list_t *input, const char *metric_name);
prediction_metric_result_t * prediction_list_extract_head(prediction_list_t *input);
void prediction_metric_result_delete(prediction_metric_result_t *pmr);
void prediction_list_delete(prediction_list_t *po);

int prediction_attach_metric(prediction_t *p, metric_t *m);
prediction_list_t *prediction_exec(prediction_t *p);
void prediction_delete(prediction_t *p);

#endif // PREDICTION_H
