#ifndef MODEL_PRIV_H
#define MODEL_PRIV_H

#include <model.h>
#include <list.h>

typedef struct {
	model_t base;

	model_exec_t exec;
	model_delete_t dtor;

	char *name;
	void *user;
} model_priv_t;

model_priv_t * model_priv(model_t *m);

model_t * model_create(model_exec_t exec, model_delete_t dtor, const char *name,
		       void *user);

#endif // MODEL_PRIV_H
