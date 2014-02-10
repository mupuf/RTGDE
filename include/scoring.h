#ifndef SCORE_H
#define SCORE_H

#include "decision.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {

} scoring_metric_t;

typedef struct {

} scoring_t;

scoring_metric_t * scoring_metric_create(scoring_t *s, const char *name, int weight);
void scoring_delete(scoring_t *s);
scoring_metric_t * scoring_metric_by_name(scoring_t *s, const char *name);
int scoring_metric_weight(scoring_metric_t *metric);
void scoring_metric_delete(scoring_metric_t *metric);

typedef float (*scoring_calc_t)(scoring_t *s, const prediction_metric_result_t *pmr,
				const graph_t *model_out);
typedef void (*scoring_dtor_t)(scoring_t *s);

scoring_t *scoring_create(scoring_calc_t calc, scoring_dtor_t dtor,
			  const char *name, void *user);
void *scoring_user(scoring_t *s);
const char *scoring_name(scoring_t *s);
int scoring_exec(scoring_t *s, decision_input_t *di);

#ifdef __cplusplus
}
#endif

#endif // SCORE_H
