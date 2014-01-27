#ifndef SCORE_PRIV_H
#define SCORE_PRIV_H

#include "scoring.h"
#include "prediction.h"

typedef float (*scoring_calc_t)(scoring_t *s, const prediction_metric_result_t *pmr, const graph_t *model_out);
typedef void (*scoring_dtor_t)(scoring_t *s);

typedef struct {
	scoring_t base;

	scoring_calc_t calc;
	scoring_dtor_t dtor;

	const char *name;
	void *user;

	struct list_head metrics;
} scoring_priv_t;

typedef struct {
	scoring_metric_t base;

	/* private declarations */
	struct list_head list;
	char *name;
	int weight;
} scoring_metric_priv_t;

scoring_t *scoring_create(scoring_calc_t calc, scoring_dtor_t dtor,
			  const char *name, void *user);
void *scoring_user(scoring_t *s);

#endif // SCORE_PRIV_H
