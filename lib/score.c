#include "score_priv.h"
#include <string.h>

static score_metric_priv_t *score_metric_priv(score_metric_t *m)
{
	return (score_metric_priv_t *)m;
}

score_priv_t *score_priv(scoring_t *s)
{
	return (score_priv_t *)s;
}

scoring_t *score_create(score_calc_t calc, score_dtor_t dtor, const char *name, void *user)
{
	score_priv_t *s_priv = malloc(sizeof(score_priv_t));
	if (!s_priv)
		return NULL;

	INIT_LIST_HEAD(&s_priv->metrics);
	s_priv->calc = calc;
	s_priv->dtor = dtor;
	s_priv->base.name = strdup(name);
	s_priv->base.user = user;

	return (scoring_t *)s_priv;
}

void score_delete(scoring_t *s)
{
	score_priv_t *s_priv = score_priv(s);
	if (s_priv->dtor)
		s_priv->dtor(s);
	free((char *)s->name);
	free(s);
}

score_metric_t * scoring_metric_create(scoring_t *s, const char *name, int weight)
{
	score_priv_t *s_priv = score_priv(s);
	score_metric_priv_t *sm_priv;
	score_metric_priv_t *pos;

	list_for_each_entry(pos, &s_priv->metrics, list) {
		if (strcmp(pos->name, name) == 0) {
			fprintf(stderr,
				"score_metric_create: "
				"Metric '%s' is already defined",
				name);
			return NULL;
		}
	}

	sm_priv = malloc(sizeof(score_metric_priv_t));
	if (!sm_priv)
		return NULL;

	INIT_LIST_HEAD(&sm_priv->list);
	sm_priv->name = strdup(name);
	sm_priv->weight = weight;

	list_add(&sm_priv->list, &s_priv->metrics);

	return (score_metric_t *)sm_priv;
}

score_metric_t *scoring_metric_by_name(scoring_t *s, const char *name)
{
	score_priv_t *s_priv = score_priv(s);
	score_metric_priv_t *pos;

	list_for_each_entry(pos, &s_priv->metrics, list) {
		if (strcmp(pos->name, name) == 0) {
			return (score_metric_t*) pos;
		}
	}
	return NULL;
}

int scoring_metric_weight(score_metric_t *metric)
{
	return score_metric_priv(metric)->weight;
}

void scoring_metric_delete(score_metric_t *metric)
{
	score_metric_priv_t *sm_priv = score_metric_priv(metric);
	list_del(&(sm_priv->list));
	free(sm_priv->name);
	free(sm_priv);
}

int scoring_exec(scoring_t *s, decision_input_t *dim)
{
	return 1;
}
