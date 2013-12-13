#include "fsm_priv.h"
#include <stddef.h>

fsm_priv_t *fsm_priv(fsm_t *fsm)
{
	return (fsm_priv_t*)fsm;
}

fsm_t *fsm_create(fsm_next_state_t next_state, fsm_dtor_t dtor, void *user)
{
	fsm_priv_t *f_priv = (fsm_priv_t *) malloc(sizeof(fsm_priv_t));
	if (!f_priv)
		return NULL;

	f_priv->next_state = next_state;
	f_priv->dtor = dtor;
	f_priv->user = user;

	return (fsm_t *)f_priv;
}

fsm_state_t *fsm_add_state(fsm_t *fsm, void *user)
{
	fsm_priv_t *f_priv = fsm_priv(fsm);
	fsm_state_priv_t *fs_priv = (fsm_state_priv_t *) malloc(sizeof(fsm_state_priv_t));
	if (!fs_priv)
		return NULL;

	fs_priv->user = user;

	list_add_tail(&fs_priv->list, &f_priv->states);

	return (fsm_state_t *)fs_priv;
}

fsm_state_t *fsm_update_state(fsm_t *fsm, const char *metric, void *value)
{
	fsm_priv_t *f_priv = fsm_priv(fsm);
	f_priv->cur = f_priv->next_state(fsm, metric, value);
	return f_priv->cur;
}

void *fsm_get_user(fsm_t *fsm)
{
	fsm_priv_t *f_priv = fsm_priv(fsm);
	return f_priv->user;
}

void fsm_delete(fsm_t *fsm)
{
	fsm_priv_t *f_priv = fsm_priv(fsm);
	fsm_state_priv_t *pos, *n;

	/* free the metrics list */
	list_for_each_entry_safe(pos, n, &f_priv->states, list) {
		list_del(&(pos->list));
		free(pos);
	}

	if (f_priv->dtor)
		f_priv->dtor(fsm);
	free(fsm);
}
