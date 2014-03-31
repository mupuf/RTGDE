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

#include "dvfs_decision.h"
#include <assert.h>
#include "utils.h"

struct ring {
	size_t size;
	size_t capacity;
	float *data;
};

enum core {
	NONE = 0,
	BIG = 1,
	LITTLE = 2
};

typedef struct {
	size_t counter;

	int64_t last_update;
	enum core cur_core;
} decision_dvfs_t;

decision_input_model_t *decision_dvfs_calc(decision_t *d, decision_input_t *di)
{
	decision_dvfs_t *dvfs = (decision_dvfs_t*)decision_user(d);
	decision_input_model_t *big, *little;

	big = decision_input_model_get_by_name(di, "core-BIG");
	little = decision_input_model_get_by_name(di, "core-LITTLE");

	if (!big || !little)
		return NULL;

	if (dvfs->cur_core == LITTLE) {
		dvfs->counter = 0;
		if (big->score >= (little->score * 1.2))
			dvfs->cur_core = BIG;
	} else {
		if (big->score <= (little->score * 1.1))
			dvfs->counter++;
		else if (big->score <= (little->score * 0.8))
			dvfs->counter = 0;

		if (dvfs->counter > 5)
			dvfs->cur_core = LITTLE;
	}

	return dvfs->cur_core == LITTLE ? little : big;
}

void decision_dvfs_dtor(decision_t *d)
{
	free(decision_user(d));
}

decision_t * decision_dvfs_create()
{
	decision_dvfs_t *dvfs = malloc(sizeof(decision_dvfs_t));
	if (!dvfs)
		return NULL;

	dvfs->counter = 0;

	dvfs->last_update = 0;
	dvfs->cur_core = LITTLE;

	return decision_create(decision_dvfs_calc, decision_dvfs_dtor,
			       "decision_dvfs", (void *)dvfs);
}
