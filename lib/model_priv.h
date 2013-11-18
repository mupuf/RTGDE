#ifndef MODEL_PRIV_H
#define MODEL_PRIV_H

#include <model.h>
#include <list.h>

typedef struct {
	model_output_metric_t base;
	struct list_head list;
} model_output_metric_priv_t;

typedef struct {
	model_output_t base;

} model_output_priv_t;

typedef struct {
	model_t base;
} model_priv_t;

#endif // MODEL_PRIV_H
