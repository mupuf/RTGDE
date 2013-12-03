#ifndef SCORE_PRIV_H
#define SCORE_PRIV_H

#include "score.h"
#include "prediction.h"


typedef int (*score_calc_t)(const prediction_t *p, const graph_t *model_out);

typedef struct {
	score_t base;

	score_calc_t calc;

	void *user;
} score_priv_t;

typedef struct {
	score_metric_t base;

	int weight;

	score_priv_t *s_priv;
} score_metric_priv_t;

score_priv_t *score_priv(score_t *s);
score_t * score_create(const char *name, const score_calc_t calc, void *user);

#endif // SCORE_PRIV_H
