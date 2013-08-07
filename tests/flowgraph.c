#include <rtgde.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main(int argc, char **argv)
{
	int i;

	metric_t * m = metric_create("throughput", 10);

	flowgraph_t *f = flowgraph_create("nif selector", 1000000);

	int ret = flowgraph_attach_metric(f, m);
	if (ret)
		return -1;

	rtgde_start(f);

	for (i = 0; i < 15; i++) {
		metric_update(m, /*clock_read_us()*/i, i);
		metric_print_history(m);
		usleep(1000);
	}

	flowgraph_teardown(f);

	metric_delete(m);

	return 0;
}
