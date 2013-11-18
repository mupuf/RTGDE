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

typedef struct {
	struct list_head metrics;
} prediction_output_t;

typedef int (*prediction_check_t)(prediction_t *p);
typedef prediction_output_t *(*prediction_exec_t)(prediction_t *p, prediction_output_t *po);
typedef void (*prediction_delete_t)(prediction_t *p);

prediction_t * prediction_create(prediction_check_t check,
				 prediction_exec_t exec,
				 prediction_delete_t dtor,
				 void *user);

int prediction_attach_metric(prediction_t *p, metric_t *m);
prediction_output_t *prediction_exec(prediction_t *p);
void prediction_output_delete(prediction_output_t *po);
void prediction_delete(prediction_t *p);

#endif // PREDICTION_H
