#ifndef SCORE_PRIV_H
#define SCORE_PRIV_H

#include "score.h"

typedef struct {
	score_metric_t base;

	int weight;

	score_priv_t *score_o;
} score_metric_t;

typedef struct {
	score_t base;

	score_calc_t calc;

	void *user;
} score_priv_t;

typedef int (*score_calc_t)(const prediction_t *p, const graph_t *model_out);

score_t * score_create(const char *name, const score_calc_t calc, void *user);
score_metric_t * score_new_metric(const score_t *s, );

#endif // SCORE_PRIV_H
