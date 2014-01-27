#include "scoring.h"
#include "../scoring_priv.h"
#include "scoring/simple.h"
#include <assert.h>

#define MIN2(A,B)       ((A)<(B)?(A):(B))
#define MIN3(A,B,C)     (MIN2(MIN2((A),(B)),(C)))
#define MIN4(A,B,C,D)   (MIN2(MIN3((A),(B),(C)),(D)))

typedef struct {
	score_simple_style_t inverted;
} score_simple_t;

float score_simple_calc(scoring_t *s, const prediction_metric_result_t *pmr, const graph_t *model_out)
{
	score_simple_t *simple = (score_simple_t*) scoring_user(s);
	const sample_t *h, *nh, *a, *na, *l, *nl, *m, *nm;
	sample_time_t last_update = 0, start_time;
	double score = 0.0, score_seg = 0.0;

	h = graph_read_first(pmr->high);
	a = graph_read_first(pmr->average);
	l = graph_read_first(pmr->low);
	m = graph_read_first(model_out);

	assert(h->time == 0);
	assert(a->time == 0);
	assert(l->time == 0);
	assert(m->time == 0);
	start_time = 0;

	do {
		nh = graph_read_next(pmr->high, h);
		na = graph_read_next(pmr->average, a);
		nl = graph_read_next(pmr->low, l);
		nm = graph_read_next(model_out, m);

		if (!nh || !na || !nl || !nm)
			break;

		/* calc the score on this segment */
		sample_time_t min_time = MIN4(nh->time, na->time, nl->time, nm->time);
		sample_time_t len = min_time - last_update;
		assert(min_time > last_update);

		if (m->value >= h->value)
			score_seg = 1.0;
		else if (m->value >= a->value)
			score_seg = 0.5 + (((float)(m->value - a->value)) / (h->value - a->value))/2.0;
		else if (m->value <= l->value)
			score_seg = 0.0;
		else if (m->value < a->value)
			score_seg = (((float)(m->value - l->value)) / (a->value - l->value))/2.0;

		fprintf(stderr, "p (%i, %i, %i) m (%i): score_seg = %f\n",
			h->value, a->value, l->value, m->value, score_seg);

		if (simple->inverted)
			score_seg = 1.0 - score_seg;

		/* calc score */
		score += score_seg * len;

		/* move to the next point */
		if (nh->time == min_time)
			h = nh;
		if (na->time == min_time)
			a = na;
		if (nl->time == min_time)
			l = nl;
		if (nm->time == min_time)
			m = nm;
		last_update = min_time;
	} while (nh && na && nl && nm);

	/* normalize the score */
	score /= (last_update - start_time);
	assert(score <= 1.0);
	fprintf(stderr, "=> score = %f\n", score);

	return score;
}

void score_simple_dtor(scoring_t *s)
{
	free(scoring_user(s));
}

scoring_t * score_simple_create(score_simple_style_t inverted)
{
	score_simple_t *simple = malloc(sizeof(score_simple_t));
	if (!simple)
		return NULL;

	simple->inverted = inverted;

	return scoring_create(score_simple_calc, score_simple_dtor,
			    "score_simple", (void *)simple);
}


