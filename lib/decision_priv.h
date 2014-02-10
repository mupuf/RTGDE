#ifndef DECISION_PRIV_H
#define DECISION_PRIV_H

#include "decision.h"

typedef struct {
	decision_t base;

	void *user_cb_data;

	decision_exec_t exec;
	decision_dtor_t dtor;

	const char *name;
	void *user;
} decision_priv_t;

#endif // DECISION_PRIV_H
