#ifndef MODEL_H
#define MODEL_H

#include <graph.h>
#include <list.h>

typedef struct {
	struct list_head list;

	const char *name;

	/* prediction output */
	graph_t *high;
	graph_t *average;
	graph_t *low;
} model_input_t;

typedef struct {
	struct list_head list;

	model_input_t *input;
	graph_t *output;
	uint32_t score;
} model_output_metric_t;

typedef struct {
	uint32_t score;

	struct list_head metrics;
} model_output_t;

typedef struct {

} model_t;

typedef model_output_t *(*model_exec_t)(model_t *p, const model_input_t *m, void *user);

model_t * model_create(model_exec_t exec, void *user);
model_output_t *model_exec(model_t *p);
void model_delete(model_t *p);

#endif // MODEL_H
