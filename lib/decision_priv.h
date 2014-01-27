#ifndef DECISION_PRIV_H
#define DECISION_PRIV_H

#include "decision.h"

typedef void (*decision_exec_t)(decision_t *d, decision_input_t *di);
typedef void (*decision_dtor_t)(decision_t *d);

typedef struct {
	decision_t base;

	decision_callback_t user_cb;
	void *user_cb_data;

	decision_exec_t exec;
	decision_dtor_t dtor;

	const char *name;
	void *user;
} decision_priv_t;

decision_t *decision_create(decision_exec_t exec, decision_dtor_t dtor,
			    decision_callback_t user_cb, void *user_cb_data,
			    const char *name, void *user);
void *decision_user(decision_t *d);
void decision_call_user_cb(decision_t *d, model_t *m);

#endif // DECISION_PRIV_H
