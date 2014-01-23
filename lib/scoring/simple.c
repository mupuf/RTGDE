#include "score.h"
#include "../scoring_priv.h"

typedef struct {

} score_simple_t;

float score_simple_calc(scoring_t *s, const prediction_metric_result_t *pmr, const graph_t *model_out)
{
	return 1.0;
}

void score_simple_dtor(scoring_t *s)
{
	free(s->user);
}

scoring_t * score_simple_create()
{
	score_simple_t *simple = malloc(sizeof(score_simple_t));
	if (!simple)
		return NULL;

	return scoring_create(score_simple_calc, score_simple_dtor,
			    "score_simple", (void *)simple);
}


