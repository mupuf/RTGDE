#include <rtgde.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <predictions/simple.h>
#include <predictions/average.h>
#include <models/dummy.h>
#include <scoring/simple.h>

int64_t relative_time_us()
{
	static int64_t boot_time = 0;
	if (boot_time == 0)
		boot_time = clock_read_us();
	return clock_read_us() - boot_time;
}

int main(int argc, char **argv)
{
	int i;

	prediction_t * mp = prediction_average_create(1000000, 2);
	assert(mp);

	metric_t * me1 = metric_create("throughput", 10);
	assert(me1);
	metric_t * me2 = metric_create("latency", 10);
	assert(me2);

	metric_t * me3 = metric_create("lol", 10);
	assert(me3);

	assert(!prediction_attach_metric(mp, me1));
	assert(!prediction_attach_metric(mp, me2));
	assert(!prediction_attach_metric(mp, me3));

	scoring_t * scoring = score_simple_create();
	assert(scoring);

	assert(scoring_metric_create(scoring, "throughput", 2));

	flowgraph_t *f = flowgraph_create("nif selector", scoring, 1000000);

	assert(!flowgraph_attach_prediction(f, mp));

	model_t * m = model_dummy_create();
	assert(m);

	assert(!flowgraph_attach_model(f, m));

	prediction_output_csv(mp, "pred_average_%s_%i.csv");

	rtgde_start(f);

	for (i = 0; i < 10; i++) {
		metric_update(me1, relative_time_us(), i);
		metric_update(me2, relative_time_us(), i+1000);
		metric_update(me3, relative_time_us(), i+2000);
		usleep(500000);
	}

	flowgraph_teardown(f);

	model_delete(m);

	prediction_delete(mp);

	metric_delete(me3);
	metric_delete(me2);
	metric_delete(me1);

	return 0;
}
