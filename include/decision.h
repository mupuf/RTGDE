#ifndef DECISION_H
#define DECISION_H

#include "list.h"
#include "model.h"
#include "prediction.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	const char *name;
	void *user;
} decision_t;

typedef decision_input_model_t *(*decision_exec_t)(decision_t *d, decision_input_t *di);
typedef void (*decision_dtor_t)(decision_t *d);

decision_t *decision_create(decision_exec_t exec, decision_dtor_t dtor,
			    const char *name, void *user);
void *decision_user(decision_t *d);
void decision_call_user_cb(decision_t *d, model_t *m);
const char *decision_name(decision_t *d);
decision_input_model_t *decision_exec(decision_t *d, decision_input_t *di);
void decision_delete(decision_t *d);


#ifdef __cplusplus
}
#endif

#endif // DECISION_H
