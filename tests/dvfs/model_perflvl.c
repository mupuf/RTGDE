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

#include "model_perflvl.h"
#include "model.h"
#include <assert.h>
#include <inttypes.h>

typedef struct {
	size_t perflvl_cnt;
	model_core_set_perflvl_t *perflvls;
	size_t perflvl;
} model_core_t;


model_core_t * model_core(model_t* m)
{
	return (model_core_t*) model_user(m);
}

size_t selectPerfLvl(model_core_t *core, sample_value_t perf)
{
	size_t i;
	for (i = 0; i < core->perflvl_cnt; i++) {
		if (core->perflvls[i].perf_impact * 100 >= perf)
			return i;
	}

	return core->perflvl_cnt - 1;
}

decision_input_model_t * model_core_exec(model_t *m, prediction_list_t *input)
{
	model_core_t *core = model_core(m);
	prediction_metric_result_t *cpu_usage, *pwr_constraint;
	const sample_t *s_avg, *s_high;
	decision_input_metric_t *di_metric_usage, *di_metric_pwr;
	decision_input_model_t *dim = NULL;

	cpu_usage = prediction_list_find(input, "CPU usage");
	if (!cpu_usage) {
		fprintf(stderr, "model_core: prediction 'CPU_usage' has not been found!\n");
		return NULL;
	}

	pwr_constraint = prediction_list_find(input, "power");
	if (!pwr_constraint) {
		fprintf(stderr, "model_core: constraint 'power' has not been found!\n");
		return NULL;
	}

	graph_t * o_cpu_usage = graph_create();
	if (!o_cpu_usage)
		 return NULL;

	graph_t * o_pwr = graph_create();
	if (!o_pwr)
		 return NULL;

	s_avg = graph_read_first(cpu_usage->average);
	s_high = graph_read_first(cpu_usage->high);

	while (s_avg && s_high) {
		size_t p_avg, p_high, p_chosen;

		p_avg = selectPerfLvl(core, s_avg->value);
		p_high = selectPerfLvl(core, s_high->value);

		//if (p_avg == p_high)
		p_chosen = p_high;
		core->perflvl = p_chosen;

		sample_value_t p_impact = core->perflvls[p_chosen].perf_impact * 100;
		graph_add_point(o_cpu_usage, s_avg->time, p_impact);

		float perf_possible = (core->perflvls[p_chosen].perf_impact * 100);
		float predicted_cpu_usage = 1.0 - ((perf_possible - s_avg->value) / perf_possible);
		/* do not clamp the predicted_cpu_usage to 1 in order to penalize for insufficient performance */
		float pwr = core->perflvls[p_chosen].static_pwr_mW + predicted_cpu_usage * core->perflvls[p_chosen].dyn_pwr_mW;
		fprintf(stderr, "%s: pwr = %f (static = %f, dyn = %f), perf_possible = %f, predicted_cpu_usage = %f\n",
			model_name(m), pwr,
			core->perflvls[p_chosen].static_pwr_mW,
			predicted_cpu_usage * core->perflvls[p_chosen].dyn_pwr_mW,
			perf_possible, predicted_cpu_usage);
		graph_add_point(o_pwr, s_avg->time, pwr);

		s_avg = graph_read_next(cpu_usage->average, s_avg);
		s_high = graph_read_next(cpu_usage->high, s_high);
	}

	dim = decision_input_model_create(m);
	if (!dim)
		 return NULL;

	di_metric_usage = decision_input_metric_create("CPU usage",
						       prediction_metric_result_copy(cpu_usage),
						       o_cpu_usage);
	di_metric_pwr = decision_input_metric_create("Power consumption",
						     prediction_metric_result_copy(pwr_constraint),
						     o_pwr);

	decision_input_model_add_metric(dim, di_metric_usage);
	decision_input_model_add_metric(dim, di_metric_pwr);

	return dim;
}

void model_core_delete(model_t *m)
{
	model_core_t *core = model_core(m);

	free(core->perflvls);
	free(model_core(m));
}

model_t *model_core_set_create(const char *name, const model_core_set_perflvl_t *perflvls, size_t perflvl_cnt)
{
	size_t i;

	if (!perflvl_cnt)
		return 0;

	model_core_t *core = malloc(sizeof(model_core_t));
	if (!core)
		return NULL;

	core->perflvls = calloc(perflvl_cnt, sizeof(model_core_set_perflvl_t));
	core->perflvl_cnt = perflvl_cnt;
	for (i = 0; i < perflvl_cnt; i++) {
		core->perflvls[i] = perflvls[i];
	}
	core->perflvl = 0;

	return model_create(model_core_exec, model_core_delete,
			    name, (void *)core);
}

size_t model_core_current_perflvl(const model_t *m)
{
	model_core_t *core = model_core((model_t *)m);
	return core->perflvl;
}
