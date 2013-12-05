#ifndef FSM_PRIV_H
#define FSM_PRIV_H

#include "fsm.h"
#include "list.h"

typedef struct {
	fsm_state_t base;
	struct list_head list;

	void *user;
} fsm_state_priv_t;

typedef struct {
	fsm_t base;

	struct list_head states;
	fsm_state_t *cur;

	next_state_t next_state;
	void *user;
} fsm_priv_t;


#endif // FSM_PRIV_H
