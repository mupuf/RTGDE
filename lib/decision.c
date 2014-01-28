#include "decision_priv.h"
#include <string.h>

decision_priv_t *decision_priv(decision_t *s)
{
	return (decision_priv_t *)s;
}

decision_t *decision_create(decision_exec_t exec, decision_dtor_t dtor,
			    decision_callback_t user_cb, void *user_cb_data,
			    const char *name, void *user)
{
	decision_priv_t *d_priv = malloc(sizeof(decision_priv_t));
	if (!d_priv)
		return NULL;

	d_priv->user_cb = user_cb;
	d_priv->user_cb_data = user_cb_data;
	d_priv->exec = exec;
	d_priv->dtor = dtor;
	d_priv->name = strdup(name);
	d_priv->user = user;

	return (decision_t *)d_priv;
}

const char *decision_name(decision_t *d)
{
	return decision_priv(d)->name;
}

void *decision_user(decision_t *d)
{
	return decision_priv(d)->user;
}

void decision_exec(decision_t *d, decision_input_t *di)
{
	decision_priv_t *d_priv = decision_priv(d);
	if (d_priv->exec)
		d_priv->exec(d, di);
}

void decision_call_user_cb(decision_t *d, model_t *m)
{
	/* TODO: run in another thread! */
	decision_priv(d)->user_cb(d, m);
}

void decision_delete(decision_t *d)
{
	decision_priv_t *d_priv = decision_priv(d);
	if (d_priv->dtor)
		d_priv->dtor(d);
	free((char *)d_priv->name);
	free(d);
}
