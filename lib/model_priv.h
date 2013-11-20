#ifndef MODEL_PRIV_H
#define MODEL_PRIV_H

#include <model.h>
#include <list.h>

typedef struct {
	model_output_metric_t base;
	struct list_head list;
} model_output_metric_priv_t;

typedef struct {
	model_output_t base;
} model_output_priv_t;

typedef struct {
	model_t base;

	model_exec_t exec;
	model_delete_t dtor;

	void *user;
} model_priv_t;

model_priv_t * model_priv(model_t *m);
model_output_t * mode_output_create();

model_t * model_create(model_exec_t exec, model_delete_t dtor, void *user);

#endif // MODEL_PRIV_H
