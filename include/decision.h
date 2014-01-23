#ifndef DECISION_H
#define DECISION_H

#include "list.h"
#include "prediction.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef uint32_t score_t;

typedef struct {
	struct list_head list;

	prediction_metric_result_t *prediction;
	graph_t *output;
	score_t score;
} decision_input_metric_t;

typedef struct {
	struct list_head models;
} decision_input_t;

typedef struct {
	decision_input_t *parent;
	struct list_head list;
	struct list_head predictions;
	score_t score;
} decision_input_model_t;


decision_input_t * decision_input_create();
decision_input_model_t * decision_input_model_create();
decision_input_metric_t * decision_input_metric_create(prediction_metric_result_t *prediction,
						       graph_t *output);
void decision_input_add_model(decision_input_t *di, decision_input_model_t *dim);
decision_input_model_t *decision_input_model_get_first(decision_input_t *di);
decision_input_model_t *decision_input_model_get_next(decision_input_model_t *dim);
void decision_input_model_add_metric(decision_input_model_t *dim,
				     decision_input_metric_t *di_metric);
decision_input_metric_t *decision_input_metric_from_name(decision_input_model_t *dim,
							 const char *name);
void decision_input_model_delete(decision_input_model_t *dim);
void decision_input_delete(decision_input_t *di);

#ifdef __cplusplus
}
#endif

#endif // DECISION_H
