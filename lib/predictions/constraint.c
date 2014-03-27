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

#include "prediction.h"
#include "../prediction_priv.h"
#include <string.h>

typedef struct {
	sample_time_t prediction_length;
	char *constraint_name;
	char *unit;
	sample_value_t low;
	sample_value_t avg;
	sample_value_t high;
	score_simple_style_t scoring_style;
} prediction_constraint_t;

prediction_constraint_t * prediction_constraint(prediction_t* p)
{
	return (prediction_constraint_t*) p->user;
}

int prediction_constraint_check(prediction_t *p)
{
	return 0;
}

prediction_list_t *prediction_constraint_exec(prediction_t *p)
{
	prediction_constraint_t *constraint = prediction_constraint(p);
	prediction_metric_result_t *r;

	prediction_list_t *pl = prediction_list_create();
	if (!pl)
		return NULL;

	r = prediction_metric_result_create(constraint->constraint_name,
					    constraint->unit,
					    constraint->scoring_style);

	r->usage_hint = pmr_usage_constraint;

	graph_add_point((graph_t *)r->high, 0, constraint->high);
	graph_add_point((graph_t *)r->high, constraint->prediction_length,
			constraint->high);

	graph_add_point((graph_t *)r->average, 0, constraint->avg);
	graph_add_point((graph_t *)r->average, constraint->prediction_length,
			constraint->avg);

	graph_add_point((graph_t *)r->low, 0, constraint->low);
	graph_add_point((graph_t *)r->low, constraint->prediction_length,
			constraint->low);

	prediction_list_append(pl, r);

	return pl;
}

void prediction_constraint_dtor(prediction_t *p)
{
	prediction_constraint_t * constraint = prediction_constraint(p);
	free(constraint->constraint_name);
	free(constraint->unit);
	free(constraint);
}

prediction_t * prediction_constraint_create(const char *constraint_name,
					    const char *unit,
					    sample_time_t prediction_length,
					    sample_value_t low,
					    sample_value_t avg,
					    sample_value_t high,
					    score_simple_style_t scoring_style)
{
	prediction_constraint_t *constraint = malloc(sizeof(prediction_constraint_t));
	if (!constraint)
		return NULL;

	constraint->prediction_length = prediction_length;
	constraint->constraint_name = strdup(constraint_name);
	constraint->unit = strdup(unit);
	constraint->low = low;
	constraint->avg = avg;
	constraint->high = high;
	constraint->scoring_style = scoring_style;
	return prediction_create(prediction_constraint_check,
				 prediction_constraint_exec,
				 prediction_constraint_dtor,
				 "constraint",
				 (void *)constraint);
}
