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

typedef int (*prediction_check_t)(prediction_t *p);
typedef prediction_list_t *(*prediction_exec_t)(prediction_t *p);
typedef void (*prediction_delete_t)(prediction_t *p);

typedef struct {
	prediction_t base;

	struct list_head metrics;
	uint32_t metrics_count;

	prediction_check_t check;
	prediction_exec_t exec;
	prediction_delete_t dtor;

	/* CSV output */
	char * csv_filename_format;
	uint64_t prediction_count;
} prediction_priv_t;

prediction_t * prediction_create(prediction_check_t check,
				 prediction_exec_t exec,
				 prediction_delete_t dtor,
				 void *user);

prediction_priv_t * prediction_priv(prediction_t* p);
prediction_metric_result_t *prediction_metric_result_create(const char *name);
void prediction_list_append(prediction_list_t *pl, prediction_metric_result_t *pmr);
uint32_t prediction_metrics_count(prediction_t* p);

#endif // PREDICTION_PRIV_H
