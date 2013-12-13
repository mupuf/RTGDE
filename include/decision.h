#ifndef DECISION_H
#define DECISION_H

#include "list.h"
#include "prediction.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	struct list_head list;

	prediction_metric_result_t *prediction;
	graph_t *output;
	uint32_t score;
} decision_input_metric_t;

typedef struct {
	struct list_head list;
	struct list_head predictions;
} decision_input_model_t;

typedef struct {
	struct list_head models;
} decision_input_t;

decision_input_t * decision_input_create();
decision_input_model_t * decision_input_model_create();
decision_input_metric_t * decision_input_metric_create(prediction_metric_result_t *prediction,
						       graph_t *output, int score);
void decision_input_add_model(decision_input_t *di, decision_input_model_t *dim);
void decision_input_model_add_metric(decision_input_model_t *dim, decision_input_metric_t *di_metric);
void decision_input_model_delete(decision_input_model_t *dim);
void decision_input_delete(decision_input_t *di);

#ifdef __cplusplus
}
#endif

#endif // DECISION_H
