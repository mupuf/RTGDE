#ifndef GRAPH_PRIV_H
#define GRAPH_PRIV_H

#include "graph.h"
#include <stdlib.h>
#include "list.h"

typedef struct {
	sample_t base;

	/* private declarations */
	struct list_head list;
} graph_point_priv_t;

typedef struct {
	graph_t base;

	/* private declarations */
	uint64_t time_frame_us;
	uint16_t point_count;

	struct list_head point_lst;
} graph_priv_t;

graph_point_priv_t *graph_point_priv(const sample_t *p);
graph_priv_t *graph_priv(const graph_t *g);



#endif // GRAPH_PRIV_H
