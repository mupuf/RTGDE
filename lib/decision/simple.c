#include "decision.h"
#include "../decision_priv.h"
#include "decision/simple.h"
#include <assert.h>

typedef struct {

} decision_simple_t;

void decision_simple_calc(decision_t *d, decision_input_t *di)
{
	decision_input_model_t *dim, *best = NULL;
	score_t best_score = 0;

	dim = decision_input_model_get_first(di);
	while (dim) {
		if (dim->score > best_score) {
			best = dim;
			best_score = dim->score;
		}

		dim = decision_input_model_get_next(dim);
	}

	decision_call_user_cb(d, best->model); // Get the model!
}

void decision_simple_dtor(decision_t *d)
{
	free(decision_user(d));
}

decision_t * decision_simple_create(decision_callback_t user_cb, void *user_cb_data)
{
	decision_simple_t *simple = malloc(sizeof(decision_simple_t));
	if (!simple)
		return NULL;

	return decision_create(decision_simple_calc, decision_simple_dtor,
			       user_cb, user_cb_data,
			       "decision_simple", (void *)simple);
}
