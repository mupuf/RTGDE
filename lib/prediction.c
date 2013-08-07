#include "prediction_priv.h"

typedef struct {
	metric_t *base;

	/* private declarations */
	struct list_head list;
} prediction_metric_t;

prediction_priv_t * prediction_priv(prediction_t* p)
{
	return (prediction_priv_t*) p;
}

prediction_t * prediction_create(prediction_attach_metric_t metric,
				 prediction_model_check_t check,
				 prediction_model_exec_t exec,
				 void *user)
{
	prediction_priv_t *p_priv = malloc(sizeof(prediction_priv_t));
	if (!p_priv)
		return NULL;

	INIT_LIST_HEAD(&p_priv->inputs);
	p_priv->metric = metric;
	p_priv->check = check;
	p_priv->exec = exec;
	p_priv->user = user;

	return (prediction_t *)p_priv;
}

model_input_t *prediction_exec(prediction_t *p)
{
	prediction_priv_t *p_priv = prediction_priv(p);

	return p_priv->exec(p, p_priv->user);
}

void prediction_delete(prediction_t *p)
{
	prediction_priv_t *p_priv = prediction_priv(p);
	prediction_metric_t *pos, *n;

	/* free the metrics list */
	list_for_each_entry_safe(pos, n, &p_priv->inputs, list) {
		list_del(&(pos->list));
		free(pos);
	}

}

int prediction_attach_metric(prediction_t *p, metric_t * m)
{
	prediction_metric_t *pm = malloc(sizeof(prediction_metric_t));
	if (!pm)
		return -1;

	pm->base = m;

	prediction_priv_t *p_priv = prediction_priv(p);
	list_add_tail(&pm->list, &p_priv->inputs);

	return 0;
}
