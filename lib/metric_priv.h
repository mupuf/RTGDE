#ifndef METRIC_PRIV_H
#define METRIC_PRIV_H

#include <pthread.h>
#include "metric.h"
#include "sample.h"

typedef struct {
	metric_t base;

	const char *name;

	/* history */
	pthread_mutex_t history_mutex;
	history_size_t history_size;
	sample_t *ring;
	history_size_t put;
	history_size_t get;
} metric_priv_t;

metric_priv_t * metric_priv(metric_t* m);

#endif // METRIC_PRIV_H
