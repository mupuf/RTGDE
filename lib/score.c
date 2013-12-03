#include "score_priv.h"

score_priv_t *score_priv(score_t *s)
{
	return (score_priv_t *)s;
}

score_t *score_create(const char *name, score_calc_t calc, void *user)
{
	score_priv_t *s_priv = malloc(sizeof(score_priv_t));
	if (!s_priv)
		return NULL;

	s_priv->calc = calc;
	s_priv->user = user;

	return (score_t *)s_priv;
}

score_metric_t * score_metric_create(score_t *s, int weight)
{
	score_priv_t *s_priv = score_priv(s);
	score_metric_priv_t *sm_priv = malloc(sizeof(score_metric_priv_t));
	if (!sm_priv)
		return NULL;

	sm_priv->weight = weight;
	sm_priv->s_priv = s_priv;

	return NULL;
}
