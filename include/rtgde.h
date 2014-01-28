#ifndef RTGDE_H
#define RTGDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "metric.h"
#include "prediction.h"
#include "model.h"
#include "scoring.h"
#include "decision.h"

typedef struct {
	const char *name;
} flowgraph_t;

typedef void (*flowgraph_callback_t)(flowgraph_t *f, decision_input_t *di, model_t *m);

flowgraph_t *flowgraph_create(const char *name, scoring_t *s, decision_t *d,
			      flowgraph_callback_t user_cb, void *user_cb_data,
			      uint64_t update_period_ns);
void flowgraph_teardown(flowgraph_t *f);

int64_t clock_read_us();

int flowgraph_attach_prediction(flowgraph_t *f, prediction_t * p);
int flowgraph_detach_prediction(flowgraph_t *f, prediction_t * p);

int flowgraph_attach_model(flowgraph_t *f, model_t * m);
int flowgraph_detach_model(flowgraph_t *f, model_t * m);

int rtgde_start(flowgraph_t *f);
int rtgde_stop(flowgraph_t *f);

#ifdef __cplusplus
}
#endif

#endif // RTGDE_H
