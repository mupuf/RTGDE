#include "score_priv.h"

score_priv_t *score_create(const char *name, score_calc_t calc, void *user)
{
	prediction_priv_t *p_priv = malloc(sizeof(prediction_priv_t));
	if (!p_priv)
		return NULL;

	INIT_LIST_HEAD(&p_priv->metrics);
	p_priv->check = check;
	p_priv->exec = exec;
	p_priv->dtor = dtor;
	p_priv->base.user = user;

	return (prediction_t *)p_priv;
}

score_metric_t * score_new_metric()
{

}
