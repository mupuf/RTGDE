#ifndef MODEL_H
#define MODEL_H

#include "prediction.h"

typedef struct {
	struct list_head list;

	prediction_output_t *prediction;
	graph_t *output;
	uint32_t score;
} model_output_metric_t;

typedef struct {
	uint32_t score;

	struct list_head metrics;
} model_output_t;

typedef struct {

} model_t;

typedef model_output_t *(*model_exec_t)(model_t *p, const prediction_output_t *m, void *user);

model_t * model_create(model_exec_t exec, void *user);
model_output_t *model_exec(model_t *p);
void model_delete(model_t *p);

#endif // MODEL_H
