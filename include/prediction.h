#ifndef PREDICTION_H
#define PREDICTION_H

#include "model.h"
#include "metric.h"

typedef struct {

} prediction_t;

typedef int (*prediction_attach_metric_t)(prediction_t *p, metric_t *m);
typedef int (*prediction_model_check_t)(prediction_t *p);
typedef model_input_t *(*prediction_model_exec_t)(prediction_t *p, void *user);

prediction_t * prediction_create(prediction_attach_metric_t metric,
				 prediction_model_check_t check,
				 prediction_model_exec_t exec,
				 void *user);
model_input_t *prediction_exec(prediction_t *p);
void prediction_delete(prediction_t *p);

#endif // PREDICTION_H
