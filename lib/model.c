#include "model_priv.h"

model_priv_t * model_priv(model_t *m)
{
	return (model_priv_t *)m;
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

decision_input_model_t *model_exec(model_t *m, prediction_list_t * predictions)
{
	decision_input_model_t *di = model_priv(m)->exec(m, predictions);
	prediction_list_delete(predictions);
	return di;
}

void model_delete(model_t *m)
{
	model_priv_t * m_priv = model_priv(m);

	if (m_priv->dtor)
		m_priv->dtor(m);

	free(m_priv);
}
