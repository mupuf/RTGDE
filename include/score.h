#ifndef SCORE_H
#define SCORE_H

#include "decision.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {

} score_metric_t;

typedef struct {
	const char *name;
	void *user;
} scoring_t;

score_metric_t * scoring_metric_create(scoring_t *s, const char *name, int weight);
score_metric_t * scoring_metric_by_name(scoring_t *s, const char *name);
int scoring_metric_weight(score_metric_t *metric);
void scoring_metric_delete(score_metric_t *metric);

int scoring_exec(scoring_t *s, decision_input_t *dim);

#ifdef __cplusplus
}
#endif

#endif // SCORE_H
