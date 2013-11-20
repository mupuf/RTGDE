#include <rtgde.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <predictions/simple.h>
#include <models/dummy.h>

int main(int argc, char **argv)
{
	int i;

	metric_t * me1 = metric_create("throughput", 10);
	assert(me1);
	prediction_t * mp1 = prediction_simple_create(1000000);
	assert(mp1);

	metric_t * me2 = metric_create("throughput", 10);
	assert(me2);
	prediction_t * mp2 = prediction_simple_create(1000000);
	assert(me2);

	flowgraph_t *f = flowgraph_create("nif selector", 1000000);

	assert(!flowgraph_attach_prediction(f, mp1));
	assert(!flowgraph_attach_prediction(f, mp2));

	model_t * m = model_dummy_create();
	assert(m);

	assert(!flowgraph_attach_model(f, m));

	rtgde_start(f);

	for (i = 0; i < 10; i++) {
		metric_update(me1, clock_read_us(), i);
		metric_update(me2, clock_read_us(), i+1000);
		//metric_print_history(m);
		usleep(500000);
	}

	flowgraph_teardown(f);

	model_delete(m);

	prediction_delete(mp2);
	prediction_delete(mp1);

	metric_delete(me2);
	metric_delete(me1);

	return 0;
}
