#include <rtgde.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <predictions/simple.h>

int main(int argc, char **argv)
{
	int i;

	metric_t * m = metric_create("throughput", 10);
	assert(m);

	prediction_t * mp = prediction_simple_create(1000000);
	assert(mp);

	flowgraph_t *f = flowgraph_create("nif selector", 1000000);
	assert(!flowgraph_attach_prediction(f, mp));

	rtgde_start(f);

	for (i = 0; i < 100; i++) {
		metric_update(m, clock_read_us(), i);
		//metric_print_history(m);
		usleep(500000);
	}

	flowgraph_teardown(f);

	metric_delete(m);

	return 0;
}
