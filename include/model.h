#ifndef MODEL_H
#define MODEL_H

#include <graph.h>

typedef struct {
	const char *name;

	/* prediction output */
	graph_t *high;
	graph_t *average;
	graph_t *low;
} model_input_t;

typedef struct {
	model_input_t *input;
	graph_t *output;
	uint32_t score;
} model_output_metric_t;

typedef struct {
	uint32_t score;
} model_output_t;

typedef struct {
} model_t;

#endif // MODEL_H
