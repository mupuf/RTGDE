#ifndef DECISION_INPUT_H
#define DECISION_INPUT_H

#include "list.h"
#include "prediction.h"

#ifdef __cplusplus
extern "C" {
#endif

/* decision input : Move that to a different file? */
typedef float score_t;

typedef struct {
	struct list_head models;
} decision_input_t;

typedef struct {
	struct list_head list;
	decision_input_t *parent;

	model_t *model;
	struct list_head metrics;
	score_t score;
} decision_input_model_t;

typedef struct {
	struct list_head list;
	decision_input_model_t *parent;
	char *name;

	prediction_metric_result_t *prediction;
	graph_t *output;
	score_t score;
} decision_input_metric_t;

decision_input_t * decision_input_create();
decision_input_model_t * decision_input_model_create(model_t *model);
decision_input_metric_t * decision_input_metric_create(const char *name,
						       prediction_metric_result_t *prediction,
						       graph_t *output);
void decision_input_add_model(decision_input_t *di, decision_input_model_t *dim);
decision_input_model_t *decision_input_model_get_first(decision_input_t *di);
decision_input_model_t *decision_input_model_get_next(decision_input_model_t *dim);
void decision_input_model_add_metric(decision_input_model_t *dim,
				     decision_input_metric_t *di_metric);
decision_input_metric_t *decision_input_metric_from_name(decision_input_model_t *dim,
							 const char *name);
decision_input_metric_t *decision_input_metric_get_first(decision_input_model_t *dim);
decision_input_metric_t *decision_input_metric_get_next(decision_input_metric_t *di_metric);
void decision_input_model_delete(decision_input_model_t *dim);
void decision_input_delete(decision_input_t *di);

#ifdef __cplusplus
}
#endif

#endif // DECISION_INPUT_H
