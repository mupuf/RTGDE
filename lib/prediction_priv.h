#ifndef PREDICTION_PRIV_H
#define PREDICTION_PRIV_H

#include <prediction.h>
#include <model.h>
#include <list.h>

typedef struct {
	prediction_t base;

	char *name;

	struct list_head metrics;
	uint32_t metrics_count;

	prediction_check_t check;
	prediction_exec_t exec;
	prediction_delete_t dtor;
} prediction_priv_t;

#endif // PREDICTION_PRIV_H
