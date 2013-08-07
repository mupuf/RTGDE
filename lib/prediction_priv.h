#ifndef PREDICTION_PRIV_H
#define PREDICTION_PRIV_H

#include <prediction.h>
#include <model.h>
#include <list.h>

typedef struct {
	prediction_t base;

	struct list_head inputs;

	prediction_attach_metric_t metric;
	prediction_model_check_t check;
	prediction_model_exec_t exec;
	void *user;
} prediction_priv_t;

prediction_priv_t * prediction_priv(prediction_t* p);

#endif // PREDICTION_PRIV_H
