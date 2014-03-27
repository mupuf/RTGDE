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
#include <inttypes.h>

void print_point(const sample_t *p)
{
	printf("[%" PRIu64 ", %i]", p->time, p->value);
}

int main(int argc, char **argv)
{
	graph_t *g = graph_create();
	int i;

	/* test that fetching points from an empty graph will always return NULL */
	const sample_t * p = graph_read_first(g);
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
	const sample_t * p0 = graph_read_point(g, 0);
	assert(p0 != NULL);
	const sample_t * p1 = graph_read_point(g, 1);
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
