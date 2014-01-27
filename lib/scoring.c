#include "scoring_priv.h"
#include <string.h>

static scoring_metric_priv_t *score_metric_priv(scoring_metric_t *m)
{
	return (scoring_metric_priv_t *)m;
}

scoring_priv_t *score_priv(scoring_t *s)
{
	return (scoring_priv_t *)s;
}

scoring_t *scoring_create(scoring_calc_t calc, scoring_dtor_t dtor, const char *name, void *user)
{
	scoring_priv_t *s_priv = malloc(sizeof(scoring_priv_t));
	if (!s_priv)
		return NULL;

	INIT_LIST_HEAD(&s_priv->metrics);
	s_priv->calc = calc;
	s_priv->dtor = dtor;
	s_priv->name = strdup(name);
	s_priv->user = user;

	return (scoring_t *)s_priv;
}

void score_delete(scoring_t *s)
{
	scoring_priv_t *s_priv = score_priv(s);
	if (s_priv->dtor)
		s_priv->dtor(s);
	free((char *)s_priv->name);
	free(s);
}

scoring_metric_t * scoring_metric_create(scoring_t *s, const char *name, int weight)
{
	scoring_priv_t *s_priv = score_priv(s);
	scoring_metric_priv_t *sm_priv;

	if (scoring_metric_by_name(s, name)) {
		fprintf(stderr,
			"score_metric_create: Metric '%s' is already defined",
			name);
		return NULL;
	}

	sm_priv = malloc(sizeof(scoring_metric_priv_t));
	if (!sm_priv)
		return NULL;

	INIT_LIST_HEAD(&sm_priv->list);
	sm_priv->name = strdup(name);
	sm_priv->weight = weight;

	list_add(&sm_priv->list, &s_priv->metrics);

	return (scoring_metric_t *)sm_priv;
}

scoring_metric_t *scoring_metric_by_name(scoring_t *s, const char *name)
{
	scoring_priv_t *s_priv = score_priv(s);
	scoring_metric_priv_t *pos;

	list_for_each_entry(pos, &s_priv->metrics, list) {
		if (strcmp(pos->name, name) == 0) {
			return (scoring_metric_t*) pos;
		}
	}
	return NULL;
}

int scoring_metric_weight(scoring_metric_t *metric)
{
	return score_metric_priv(metric)->weight;
}

void scoring_metric_delete(scoring_metric_t *metric)
{
	scoring_metric_priv_t *sm_priv = score_metric_priv(metric);
	list_del(&(sm_priv->list));
	free(sm_priv->name);
	free(sm_priv);
}

const char *scoring_name(scoring_t *s)
{
	return score_priv(s)->name;
}

void *scoring_user(scoring_t *s)
{
	return score_priv(s)->user;
}

int scoring_exec(scoring_t *s, decision_input_t *di)
{
	scoring_priv_t *s_priv = score_priv(s);
	decision_input_model_t *dim;
	scoring_metric_priv_t *pos;
	decision_input_metric_t* m;

	/* for all models of dim */
	dim = decision_input_model_get_first(di);
	while (dim) {
		/* for all metrics registered */
		list_for_each_entry(pos, &s_priv->metrics, list) {
			/* fetch the corresponding entry from the decision input */
			m = decision_input_metric_from_name(dim, pos->name);

			/* call the calc function */
			/* score += metric_score * metric_weight */
			m->score = s_priv->calc(s, m->prediction, m->output) * pos->weight;

			/* model_score += metric_score */
			dim->score += m->score;
		}
		dim = decision_input_model_get_next(dim);
	}

	return 0;
}
