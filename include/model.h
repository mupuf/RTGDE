#ifndef MODEL_H
#define MODEL_H

#include "prediction.h"

typedef struct {
	struct list_head list;

	prediction_metric_result_t *prediction;
	graph_t *output;
	uint32_t score;
} model_output_metric_t;

typedef struct {
	struct list_head predictions;
} model_output_t;

typedef struct {

} model_t;

typedef model_output_t *(*model_exec_t)(model_t *m, prediction_list_t *prediction);
typedef void (*model_delete_t)(model_t *m);

void model_output_delete(model_output_t *mo);

model_output_t *model_exec(model_t *m, prediction_list_t * predictions);
void model_delete(model_t *m);

#endif // MODEL_H
