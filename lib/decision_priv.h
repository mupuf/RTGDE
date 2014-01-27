#ifndef DECISION_PRIV_H
#define DECISION_PRIV_H

#include "decision.h"

typedef void (*decision_calc_t)(decision_t *d, decision_input_t *di);
typedef void (*decision_dtor_t)(decision_t *d);

typedef struct {
	decision_t base;

	decision_calc_t calc;
	decision_dtor_t dtor;

	const char *name;
	void *user;
} decision_priv_t;

decision_t *decision_create(decision_calc_t calc, decision_dtor_t dtor,
			  const char *name, void *user);
void *decision_user(decision_t *d);

#endif // DECISION_PRIV_H
