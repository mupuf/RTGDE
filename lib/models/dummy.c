#include "model.h"
#include "../model_priv.h"

typedef struct {
	/* Nothing here */
} model_dummy_t;

model_dummy_t * model_dummy(model_t* m)
{
	return (model_dummy_t*) model_priv(m)->user;
}

model_output_t * model_dummy_exec(model_t *m, prediction_list_t *input)
{
	model_dummy_t *dummy = model_dummy(m);
	model_output_t *mo = mode_output_create();

	fprintf(stderr, "Heya!\n");

	return mo;
}

void model_dummy_delete(model_t *m)
{
	free(model_dummy(m));
}

model_t * model_dummy_create()
{
	model_dummy_t *dummy = malloc(sizeof(model_dummy_t));
	if (!dummy)
		return NULL;

	return model_create(model_dummy_exec, model_dummy_delete, (void *)dummy);
}
