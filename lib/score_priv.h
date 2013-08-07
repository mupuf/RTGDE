#ifndef SCORE_PRIV_H
#define SCORE_PRIV_H

#include "score.h"

typedef struct {
	score_metric_t base;

	const char *name;
	int weight;

	score_type_t type;
} score_metric_t;

score_metric_t *
score_new_metric(const char *name, int weight, score_type_t type);

#endif // SCORE_PRIV_H
