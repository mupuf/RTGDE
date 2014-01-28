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
	assert(dim->model == d->m);
	fprintf(stderr, "Callback decision from model '%p' with score %f\n",
		dim->model, dim->score);
}

int main(int argc, char **argv)
{
	int i;

	data.mp = prediction_average_create(1000000, 2);
	assert(data.mp);

	data.me1 = metric_create("throughput", 10);
	assert(data.me1);
	data.me2 = metric_create("latency", 10);
	assert(data.me2);
	data.me3 = metric_create("lol", 10);
	assert(data.me3);

	assert(!prediction_attach_metric(data.mp, data.me1));
	assert(!prediction_attach_metric(data.mp, data.me2));
	assert(!prediction_attach_metric(data.mp, data.me3));

	data.scoring = score_simple_create(scoring_normal);
	assert(data.scoring);

	assert(scoring_metric_create(data.scoring, "throughput", 2));

	data.decision = decision_simple_create(decision_callback, &data);
	assert(data.decision);

	data.f = flowgraph_create("nif selector", data.scoring, data.decision,
				  decision_callback, &data, 1000000);

	assert(!flowgraph_attach_prediction(data.f, data.mp));

	data.m = model_dummy_create();
	assert(data.m);

	assert(!flowgraph_attach_model(data.f, data.m));

	prediction_output_csv(data.mp, "pred_average_%s_%i.csv");

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
