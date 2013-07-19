#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>

#include "rtgde.h"
#include "list.h"

#define die(en, msg) \
	do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

typedef struct {
	metric_t *base;

	/* private declarations */
	struct list_head list;
} flowgraph_metric_t;

typedef struct {
	flowgraph_t base;

	/* private declarations */
	struct list_head metrics;
	pthread_t thread;
	uint64_t update_period_us;
} flowgraph_priv_t;

flowgraph_priv_t *flowgraph_priv(const flowgraph_t *p)
{
	return (flowgraph_priv_t *) p;
}

static uint64_t clock_read_us()
{
	struct timespec tp;
	if (clock_gettime(CLOCK_REALTIME, &tp))
		return 0;

	return tp.tv_sec * 1000000ULL + tp.tv_nsec / 1000;
}

static void *thread_flowgraph (void *p_data)
{
	flowgraph_priv_t *f = (flowgraph_priv_t *)p_data;
	int s;

	/* Get the clock */
	int64_t next_wakeup = clock_read_us();

	while (1) {
		next_wakeup += f->update_period_us;

		/* disable cancelation while we are taking a decision */
		s = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		if (s != 0)
			die(s, "pthread_setcancelstate");

		/* real work */

		/* re-enable cancelation */
		s = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		if (s != 0)
			die(s, "pthread_setcancelstate");

		int64_t s = next_wakeup - clock_read_us();
		if (s > 0)
			usleep(s);
		else
			fprintf(stderr,
				"Warning: thread_flowgraph missed a tick by %lli us\n",
				(long long int) -s);
	}
	return NULL;
}

flowgraph_t *flowgraph_create(const char *name, uint64_t update_period_ns)
{
	flowgraph_priv_t *f = malloc(sizeof(flowgraph_priv_t));
	if (!f)
		return NULL;

	INIT_LIST_HEAD(&f->metrics);
	f->update_period_us = update_period_ns;
	f->base.name = strdup(name);

	return (flowgraph_t *)f;
}

void flowgraph_teardown(flowgraph_t *f)
{
	rtgde_stop(f);
	free((char *)f->name);
	free(f);
}

int rtgde_start(flowgraph_t *f)
{
	flowgraph_priv_t *f_priv = flowgraph_priv(f);
	int s = pthread_create(&f_priv->thread, NULL, &thread_flowgraph, f);

	return s;
}

int rtgde_stop(flowgraph_t *f)
{
	flowgraph_priv_t *f_priv = flowgraph_priv(f);

	int s = pthread_cancel(f_priv->thread);
	if (s != 0)
		return s;

	s = pthread_join(f_priv->thread, NULL);
	if (s != 0)
		return s;

	return 0;
}
