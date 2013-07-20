#ifndef METRIC_H
#define METRIC_H

#include "graph.h"
#include "history.h"

typedef struct {
	const char *name;
} metric_t;

metric_t * metric_create(const char *name, history_size_t history_size);
void metric_update(metric_t *m, sample_time_t timestamp, sample_value_t value);
history_size_t metric_dump_history(metric_t *m, sample_t *buffer, history_size_t size);
history_size_t metric_history_size(metric_t *m);
void metric_delete(metric_t *m);

#endif // METRIC_H
