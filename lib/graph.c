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

#include "graph_priv.h"
#include <inttypes.h>

static graph_point_priv_t *graph_point_priv(const sample_t *p)
{
	return (graph_point_priv_t *)p;
}

graph_priv_t *graph_priv(const graph_t *g)
{
	return (graph_priv_t *)g;
}

graph_t * graph_create()
{
	graph_priv_t * g_priv = malloc(sizeof(graph_priv_t));

	if (!g_priv)
		return NULL;

	INIT_LIST_HEAD(&g_priv->point_lst);
	g_priv->point_count = 0;

	return (graph_t *)g_priv;
}

graph_t * graph_copy(const graph_t *g)
{
	graph_priv_t *g_priv = graph_priv(g);
	graph_t *ng = graph_create();
	int ret = 0;

	if (!ng)
		return NULL;

	graph_point_priv_t *pos;
	list_for_each_entry(pos, &g_priv->point_lst, list)
		ret |= graph_add_point(ng, pos->base.time, pos->base.value);

	if (ret) {
		graph_delete(ng);
		ng = NULL;
	}

	return ng;
}

int graph_add_point(graph_t *g, sample_time_t time, sample_value_t value)
{
	graph_point_priv_t *priv = malloc(sizeof(graph_point_priv_t));
	if (!priv)
		return -1;

	priv->base.time = time;
	priv->base.value = value;

	graph_priv_t *g_priv = graph_priv(g);
	list_add_tail(&priv->list, &g_priv->point_lst);
	g_priv->point_count++;

	return 0;
}

const sample_t * graph_read_point(const graph_t *g, graph_index_t index)
{
	graph_priv_t *g_priv = graph_priv(g);
	graph_point_priv_t *pos;

	graph_index_t i = 0;
	list_for_each_entry(pos, &g_priv->point_lst, list) {
		if (i == index)
			return (sample_t *) pos;
		i++;
	}

	return NULL;
}

const sample_t * graph_read_first(const graph_t *g)
{
	graph_priv_t *g_priv = graph_priv(g);

	if (list_empty(&g_priv->point_lst))
		return NULL;

	graph_point_priv_t *p;
	p = container_of(g_priv->point_lst.next, graph_point_priv_t, list);

	return (sample_t *)p;
}

const sample_t * graph_read_last(const graph_t *g)
{
	graph_priv_t *g_priv = graph_priv(g);

	if (list_empty(&g_priv->point_lst))
		return NULL;

	graph_point_priv_t *p;
	p = container_of(g_priv->point_lst.prev, graph_point_priv_t, list);

	return (sample_t *)p;
}

const sample_t * graph_read_next(const graph_t *g, const sample_t *point)
{
	graph_priv_t *g_priv = graph_priv(g);

	graph_point_priv_t *p_priv, *new_priv;
	p_priv = graph_point_priv(point);

	if (!p_priv)
		return NULL;

	if (!p_priv || p_priv->list.next == &g_priv->point_lst)
		return NULL;

	new_priv = container_of(p_priv->list.next, graph_point_priv_t, list);

	return (sample_t *)new_priv;
}

graph_index_t graph_point_count(const graph_t *g)
{
	graph_priv_t *g_priv = graph_priv(g);

	return g_priv->point_count;
}

graph_integral_t graph_integral(const graph_t *g, sample_time_t start,
						  sample_time_t end)
{
	graph_priv_t *g_priv = graph_priv(g);
	sample_value_t old_value = 0, old_time = start;
	graph_integral_t g_int = 0;

	graph_point_priv_t *pos;
	list_for_each_entry(pos, &g_priv->point_lst, list) {
		if (pos->base.time > start) {
			if (pos->base.time > end)
				g_int += (end - start) * old_value;
			else
				g_int += (pos->base.time - old_time) * old_value;

			if (pos->base.time >= end)
				return g_int;

			old_time = pos->base.time;
		}
		old_value = pos->base.value;
	}

	/* keep the old value and kee on integrating */
	g_int += (end - old_time) * old_value;
	return g_int;
}

void graph_print_coordinates(const graph_t *g)
{
	graph_priv_t *g_priv = graph_priv(g);

	graph_point_priv_t *pos;
	list_for_each_entry(pos, &g_priv->point_lst, list)
		printf("[%"PRIu64", %i\n", pos->base.time, pos->base.value);
	printf("\n");
}

void graph_delete(graph_t *g)
{
	graph_priv_t *g_priv = graph_priv(g);
	graph_point_priv_t *pos, *n;

	if (!g)
		return;

	list_for_each_entry_safe(pos, n, &g_priv->point_lst, list) {
		list_del(&(pos->list));
		free(pos);
	}

	free(g);
}
