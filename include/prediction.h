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

typedef struct {
	struct list_head list;

	const char *name;

	/* input */
	history_size_t hsize;
	sample_t *history;

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

prediction_metric_result_t *prediction_metric_result_create(const char *name);
const char *prediction_name(prediction_t* p);
void prediction_list_append(prediction_list_t *pl, prediction_metric_result_t *pmr);
uint32_t prediction_metrics_count(prediction_t* p);
struct list_head *prediction_metrics(prediction_t *p);
int prediction_attach_metric(prediction_t *p, metric_t *m);
prediction_list_t *prediction_exec(prediction_t *p);
void prediction_delete(prediction_t *p);

#ifdef __cplusplus
}
#endif

#endif // PREDICTION_H
