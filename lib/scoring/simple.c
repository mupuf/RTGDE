#include "score.h"
#include "../scoring_priv.h"
#include <assert.h>

#define MIN2(A,B)       ((A)<(B)?(A):(B))
#define MIN3(A,B,C)     (MIN2(MIN2((A),(B)),(C)))
#define MIN4(A,B,C,D)   (MIN2(MIN3((A),(B),(C)),(D)))

typedef struct {

} score_simple_t;

float score_simple_calc(scoring_t *s, const prediction_metric_result_t *pmr, const graph_t *model_out)
{
	const sample_t *h, *hp, *a, *ap, *l, *lp, *m, *mp;
	sample_time_t last_update = 0;

	hp = graph_read_first(pmr->high);
	ap = graph_read_first(pmr->average);
	lp = graph_read_first(pmr->low);
	mp = graph_read_first(model_out);

	assert(hp->time == 0);
	assert(ap->time == 0);
	assert(lp->time == 0);
	assert(mp->time == 0);

	do {
		h = graph_read_next(pmr->high, hp);
		a = graph_read_next(pmr->average, ap);
		l = graph_read_next(pmr->low, lp);
		m = graph_read_next(model_out, mp);

		/*  */
		sample_time_t min_time = MIN4(h->time, a->time, l->time, m->time);
		if (h->time != min_time)
			h = hp;
		if (a->time != min_time)
			a = ap;
		if (l->time != min_time)
			l = lp;
		if (m->time != min_time)
			m = mp;

		/* */
		sample_time_t len = min_time - last_update;

		/* TODO: inverted */


		last_update = min_time;
	} while (h && a && l && m);

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


