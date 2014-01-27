#ifndef SCORE_H
#define SCORE_H

#include "decision.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {

} scoring_metric_t;

typedef struct {
	const char *name;
	void *user;
} scoring_t;

scoring_metric_t * scoring_metric_create(scoring_t *s, const char *name, int weight);
scoring_metric_t * scoring_metric_by_name(scoring_t *s, const char *name);
int scoring_metric_weight(scoring_metric_t *metric);
void scoring_metric_delete(scoring_metric_t *metric);

int scoring_exec(scoring_t *s, decision_input_t *di);

#ifdef __cplusplus
}
#endif

#endif // SCORE_H
