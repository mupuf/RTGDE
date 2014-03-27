/*Copyright (c) 2014 Martin Peres
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "decision.h"
#include "../decision_priv.h"
#include "decision/simple.h"
#include <assert.h>

typedef struct {

} decision_simple_t;

decision_input_model_t *decision_simple_calc(decision_t *d, decision_input_t *di)
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

	return best;
}

void decision_simple_dtor(decision_t *d)
{
	free(decision_user(d));
}

decision_t * decision_simple_create()
{
	decision_simple_t *simple = malloc(sizeof(decision_simple_t));
	if (!simple)
		return NULL;

	return decision_create(decision_simple_calc, decision_simple_dtor,
			       "decision_simple", (void *)simple);
}
