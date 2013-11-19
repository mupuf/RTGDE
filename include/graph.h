#ifndef GRAPH_H
#define GRAPH_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include <limits.h>

#include "sample.h"

#define GRAPH_MAX_POINT_COUNT USHRT_MAX

typedef uint64_t graph_integral_t;
typedef uint16_t graph_index_t;

typedef struct {
	/* public declarations */
} graph_t;

graph_t * graph_create();
graph_t * graph_copy(const graph_t *g);

int graph_add_point(graph_t *g, sample_time_t time, sample_value_t value);
const sample_t * graph_read_point(const graph_t *g, graph_index_t index);

const sample_t * graph_read_first(const graph_t *g);
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
