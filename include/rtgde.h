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

} flowgraph_t;

typedef void (*flowgraph_callback_t)(flowgraph_t *f, decision_input_t *di,
				     decision_input_model_t *dim, void *user);

int64_t clock_read_us();

flowgraph_t *flowgraph_create(const char *name, scoring_t *s, decision_t *d,
			      flowgraph_callback_t user_cb, void *user_cb_data,
			      uint64_t update_period_ns);
void flowgraph_teardown(flowgraph_t *f);

const char * flowgraph_name(flowgraph_t *f);

int flowgraph_attach_prediction(flowgraph_t *f, prediction_t * p);
int flowgraph_detach_prediction(flowgraph_t *f, prediction_t * p);

int flowgraph_attach_model(flowgraph_t *f, model_t * m);
int flowgraph_detach_model(flowgraph_t *f, model_t * m);

int rtgde_start(flowgraph_t *f, int one_time);
int rtgde_stop(flowgraph_t *f);

#ifdef __cplusplus
}
#endif

#endif // RTGDE_H
