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

#ifndef GRAPH_H
#define GRAPH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <limits.h>

#include "sample.h"

#define GRAPH_MAX_POINT_COUNT USHRT_MAX

typedef int64_t graph_integral_t;
typedef uint16_t graph_index_t;

typedef struct {
	/* public declarations */
} graph_t;

graph_t * graph_create();
graph_t * graph_copy(const graph_t *g);

int graph_add_point(graph_t *g, sample_time_t time, sample_value_t value);
const sample_t * graph_read_point(const graph_t *g, graph_index_t index);

const sample_t * graph_read_first(const graph_t *g);
const sample_t * graph_read_last(const graph_t *g);
const sample_t * graph_read_next(const graph_t *g, const sample_t *point);

graph_index_t graph_point_count(const graph_t *g);
graph_integral_t graph_integral(const graph_t *g, sample_time_t start,
						  sample_time_t end);

void graph_print_coordinates(const graph_t *g);

void graph_delete(graph_t *g);

#ifdef __cplusplus
}
#endif

#endif // GRAPH_H
