#ifndef SCORE_PRIV_H
#define SCORE_PRIV_H

#include "score.h"
#include "prediction.h"


typedef int (*score_calc_t)(scoring_t *s, const prediction_t *p, const graph_t *model_out);
typedef void (*score_dtor_t)(scoring_t *s);

typedef struct {
	scoring_t base;

	score_calc_t calc;
	score_dtor_t dtor;

	struct list_head metrics;
} score_priv_t;

typedef struct {
	score_metric_t base;

	/* private declarations */
	struct list_head list;
	char *name;
	int weight;
} score_metric_priv_t;

score_priv_t *score_priv(scoring_t *s);
scoring_t *score_create(score_calc_t calc, score_dtor_t dtor, const char *name, void *user);

#endif // SCORE_PRIV_H
