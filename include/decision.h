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

const char *decision_name(decision_t *d);
decision_input_model_t *decision_exec(decision_t *d, decision_input_t *di);
void decision_delete(decision_t *d);


#ifdef __cplusplus
}
#endif

#endif // DECISION_H
