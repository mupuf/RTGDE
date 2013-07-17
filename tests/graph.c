#include <rtgde.h>
#include <assert.h>
#include <stdio.h>

void print_point(const graph_point_t *p)
{
	printf("[%llu, %u]", (unsigned long long)p->time, p->value);
}

int main(int argc, char **argv)
{
	graph_t *g = graph_create();
	int i;

	/* test that fetching points from an empty graph will always return NULL */
	const graph_point_t * p = graph_read_first(g);
	assert(p == NULL);
	p = graph_read_next(g, p);
	assert(p == NULL);
	for (i = 0; i < GRAPH_MAX_POINT_COUNT; i++)
		assert(graph_read_point(g, i) == NULL);

	/* test adding new points and check point_count is increasing */
	assert(graph_point_count(g) == 0);
	assert(graph_add_point(g, 1, 10) == 0);
	assert(graph_point_count(g) == 1);
	assert(graph_add_point(g, 5, 20) == 0);
	assert(graph_point_count(g) == 2);

	/* fetch the points one way and compare them with the other way */
	const graph_point_t * p0 = graph_read_point(g, 0);
	assert(p0 != NULL);
	const graph_point_t * p1 = graph_read_point(g, 1);
	assert(p1 != NULL);
	p = graph_read_first(g);
	assert(p == p0);
	p = graph_read_next(g, p);
	assert(p == p1);

	/* test integrals */
	assert(graph_integral(g, 0, 1) == 0);
	assert(graph_integral(g, 1, 2) == 10);
	assert(graph_integral(g, 1, 3) == 20);
	assert(graph_integral(g, 2, 3) == 10);
	assert(graph_integral(g, 0, 8) == 100);

	/* Check that reading out of bounds again but with a non-empty list */
	assert(graph_read_next(g, p) == NULL);
	for (i = 2; i < GRAPH_MAX_POINT_COUNT; i++)
		assert(graph_read_point(g, i) == NULL);

	graph_delete(g);

	return 0;
}
