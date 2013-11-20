#include "model_priv.h"

model_priv_t * model_priv(model_t *m)
{
	return (model_priv_t *)m;
}

model_output_t * mode_output_create()
{
	model_output_t *mo = malloc(sizeof(model_output_t));
	if (!mo)
		return NULL;

	INIT_LIST_HEAD(&mo->predictions);

	return mo;
}

void model_output_delete(model_output_t *mo)
{
	model_output_metric_t *pos, *n;

	if (!mo)
		return;

	/* free the predications and decision list */
	list_for_each_entry_safe(pos, n, &mo->predictions, list) {
		list_del(&(pos->list));
		prediction_metric_result_delete(pos->prediction);
		graph_delete((graph_t *)pos->output);
	}
	free(mo);
}

model_t * model_create(model_exec_t exec, model_delete_t dtor, void *user)
{
	model_priv_t *m_priv = malloc(sizeof(model_priv_t));
	if (!m_priv)
		return NULL;

	m_priv->user = user;
	m_priv->exec = exec;
	m_priv->dtor = dtor;

	return (model_t*) m_priv;
}

model_output_t *model_exec(model_t *m, prediction_list_t * predictions)
{
	model_output_t *mo = model_priv(m)->exec(m, predictions);
	prediction_output_delete(predictions);
	return mo;
}

void model_delete(model_t *m)
{
	model_priv_t * m_priv = model_priv(m);

	if (m_priv->dtor)
		m_priv->dtor(m);

	free(m_priv);
}
