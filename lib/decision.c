#include "decision_priv.h"
#include <string.h>

static decision_priv_t *decision_priv(decision_t *s)
{
	return (decision_priv_t *)s;
}

decision_t *decision_create(decision_exec_t exec, decision_dtor_t dtor,
			    const char *name, void *user)
{
	decision_priv_t *d_priv = malloc(sizeof(decision_priv_t));
	if (!d_priv)
		return NULL;

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

decision_input_model_t *decision_exec(decision_t *d, decision_input_t *di)
{
	decision_priv_t *d_priv = decision_priv(d);
	if (d_priv->exec)
		return d_priv->exec(d, di);
	return NULL;
}

void decision_delete(decision_t *d)
{
	decision_priv_t *d_priv = decision_priv(d);
	if (d_priv->dtor)
		d_priv->dtor(d);
	free((char *)d_priv->name);
	free(d);
}
