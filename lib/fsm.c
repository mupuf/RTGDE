#include "fsm_priv.h"
#include <stddef.h>

fsm_priv_t *fsm_priv(fsm_t *fsm)
{
	return (fsm_priv_t*)fsm;
}

fsm_t *fsm_create(next_state_t next_state, void *user)
{
	fsm_priv_t *f_priv = (fsm_priv_t *) malloc(sizeof(fsm_priv_t));
	if (!f_priv)
		return NULL;

	f_priv->next_state = next_state;
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
