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

#include <rtgde.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <predictions/simple.h>
#include <predictions/average.h>
#include <models/dummy.h>
#include <scoring/simple.h>
#include <decision/simple.h>

int64_t relative_time_us()
{
	static int64_t boot_time = 0;
	if (boot_time == 0)
		boot_time = clock_read_us();
	return clock_read_us() - boot_time;
}

struct flowgraph_data {
	prediction_t * mp;
	metric_t * me1;
	metric_t * me2;
	metric_t * me3;
	model_t * m;
	scoring_t * scoring;
	decision_t * decision;
	flowgraph_t *f;
} data;

void decision_callback(flowgraph_t *f, decision_input_t *di,
		       decision_input_model_t *dim, void *user)
{
	struct flowgraph_data *d = (struct flowgraph_data*) user;
	if (!dim) {
		fprintf(stderr, "Callback decision: no decision has been made!\n");
		return;
	}

	assert(dim->model == d->m);
	fprintf(stderr, "Callback decision from model '%p' with score %f\n",
		dim->model, dim->score);
}

void flowgraph_model_csv_cb(flowgraph_t *f, decision_input_metric_t* m,
					  const char *csv_filename)
{
	char cmd[1024];

	snprintf(cmd, sizeof(cmd),
		 "gnuplot -e \"filename='%s'\" ../gnuplot/metric_overview.plot",
		 csv_filename);
	system(cmd);

}

int main(int argc, char **argv)
{
	int i;

	data.mp = prediction_average_create(1000000, 2);
	assert(data.mp);

	data.me1 = metric_create("throughput", "bit/s", 10);
	assert(data.me1);
	data.me2 = metric_create("latency", "ms", 10);
	assert(data.me2);
	data.me3 = metric_create("lol", "toto", 10);
	assert(data.me3);

	assert(!prediction_attach_metric(data.mp, data.me1));
	assert(!prediction_attach_metric(data.mp, data.me2));
	assert(!prediction_attach_metric(data.mp, data.me3));

	data.scoring = score_simple_create();
	assert(data.scoring);

	assert(scoring_metric_create(data.scoring, "throughput", 2));

	data.decision = decision_simple_create(decision_callback, &data);
	assert(data.decision);

	data.f = flowgraph_create("nif selector", data.scoring, data.decision,
				  decision_callback, &data, 1000000);

	assert(!flowgraph_attach_prediction(data.f, data.mp));

	data.m = model_dummy_create("dummy");
	assert(data.m);

	assert(!flowgraph_attach_model(data.f, data.m));

	flowgraph_model_output_csv(data.f, "model_%s_metric_%s_%i.csv", flowgraph_model_csv_cb);

	rtgde_start(data.f, 0);

	for (i = 0; i < 10; i++) {
		metric_update(data.me1, relative_time_us(), i);
		metric_update(data.me2, relative_time_us(), i+1000);
		metric_update(data.me3, relative_time_us(), i+2000);
		usleep(500000);
	}

	flowgraph_teardown(data.f);

	model_delete(data.m);

	decision_delete(data.decision);

	scoring_delete(data.scoring);

	prediction_delete(data.mp);

	metric_delete(data.me3);
	metric_delete(data.me2);
	metric_delete(data.me1);

	return 0;
}
