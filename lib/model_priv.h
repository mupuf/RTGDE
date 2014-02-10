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

#endif // MODEL_PRIV_H
