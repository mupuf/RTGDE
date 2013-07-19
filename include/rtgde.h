#ifndef RTGDE_H
#define RTGDE_H

#ifdef __cplusplus
extern "C" {
#endif

#include "metric.h"
#include "prediction.h"
#include "model.h"
#include "score.h"
#include "decision.h"

typedef struct {
	const char *name;
} flowgraph_t;

flowgraph_t *flowgraph_create(const char *name, uint64_t update_period_ns);
void flowgraph_teardown(flowgraph_t *f);

int rtgde_start(flowgraph_t *f);
int rtgde_stop(flowgraph_t *f);

#ifdef __cplusplus
}
#endif

#endif // RTGDE_H
