#ifndef PREDICTION_PRIV_H
#define PREDICTION_PRIV_H

#include <prediction.h>
#include <model.h>
#include <list.h>

typedef struct {
	metric_t *base;

	/* private declarations */
	struct list_head list;
} prediction_metric_t;

typedef struct {
	prediction_t base;

	struct list_head metrics;

	prediction_check_t check;
	prediction_exec_t exec;
	prediction_delete_t dtor;
} prediction_priv_t;

prediction_priv_t * prediction_priv(prediction_t* p);
prediction_metric_result_t *prediction_metric_result_create(const char *name);

#endif // PREDICTION_PRIV_H
