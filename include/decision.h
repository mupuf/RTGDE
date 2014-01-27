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

typedef void (*decision_callback_t)(decision_t *d, model_t *m);

const char *decision_name(decision_t *d);
void decision_exec(decision_t *d, decision_input_t *di);
void decision_delete(decision_t *d);


#ifdef __cplusplus
}
#endif

#endif // DECISION_H
