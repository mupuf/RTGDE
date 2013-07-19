#include <rtgde.h>
#include <assert.h>
#include <stdio.h>

int main(int argc, char **argv)
{
	flowgraph_t *f = flowgraph_create("heya", 1000000);

	rtgde_start(f);
	sleep(5);
	rtgde_stop(f);
	flowgraph_teardown(f);

	return 0;
}
