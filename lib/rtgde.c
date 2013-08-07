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
	prediction_t *base;

	/* private declarations */
	struct list_head list;
} flowgraph_prediction_t;

typedef struct {
	prediction_t *base;

	/* private declarations */
	struct list_head list;
} flowgraph_model_t;

typedef struct {
	flowgraph_t base;

	/* private declarations */
	struct list_head predictions;
	struct list_head models;
	/* TODO: scoring */
	/* TODO: decision */
	pthread_t thread;
	uint64_t update_period_us;
} flowgraph_priv_t;

flowgraph_priv_t *flowgraph_priv(const flowgraph_t *f)
{
	return (flowgraph_priv_t *) f;
}

int64_t clock_read_us()
{
	struct timespec tp;
	if (clock_gettime(CLOCK_REALTIME, &tp))
		return 0;

	return tp.tv_sec * 1000000ULL + tp.tv_nsec / 1000;
}

static void execute_flow_graph(flowgraph_priv_t *f)
{
	/* reading the history of each metric */

	/* feed the metrics to the prediction */

	/* feed the ouput of prediction to each models */

	/* feed the output of each models to the scoring */

	/* take a decision */
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
		execute_flow_graph(f);

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

	INIT_LIST_HEAD(&f->predictions);
	f->update_period_us = update_period_ns;
	f->base.name = strdup(name);

	return (flowgraph_t *)f;
}

int flowgraph_attach_prediction(flowgraph_t *f, prediction_t * p)
{
	flowgraph_prediction_t *fp = malloc(sizeof(flowgraph_prediction_t));
	if (!fp)
		return -1;

	fp->base = p;

	flowgraph_priv_t *f_priv = flowgraph_priv(f);
	list_add_tail(&fp->list, &f_priv->predictions);

	return 0;
}

int flowgraph_detach_prediction(flowgraph_t *f, prediction_t * p)
{
	flowgraph_priv_t *f_priv = flowgraph_priv(f);
	flowgraph_prediction_t *pos, *n;

	/* free the metrics list */
	list_for_each_entry_safe(pos, n, &f_priv->predictions, list) {
		if (pos->base == p) {
			list_del(&(pos->list));
			free(pos);
			return 0;
		}
	}

	return -1;
}

int flowgraph_attach_model(flowgraph_t *f, model_t * m)
{
	flowgraph_model_t *fm = malloc(sizeof(flowgraph_model_t));
	if (!fm)
		return -1;

	fm->base = m;

	flowgraph_priv_t *f_priv = flowgraph_priv(f);
	list_add_tail(&fm->list, &f_priv->models);

	return 0;
}

int flowgraph_detach_model(flowgraph_t *f, model_t * m)
{
	flowgraph_priv_t *f_priv = flowgraph_priv(f);
	flowgraph_model_t *pos, *n;

	/* free the metrics list */
	list_for_each_entry_safe(pos, n, &f_priv->models, list) {
		if (pos->base == m) {
			list_del(&(pos->list));
			free(pos);
			return 0;
		}
	}

	return -1;
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

void flowgraph_teardown(flowgraph_t *f)
{
	flowgraph_priv_t *f_priv = flowgraph_priv(f);
	flowgraph_prediction_t *pos_p, *n_p;
	flowgraph_model_t *pos_m, *n_m;

	rtgde_stop(f);
	free((char *)f->name);

	/* free the prediction list */
	list_for_each_entry_safe(pos_p, n_p, &f_priv->predictions, list) {
		list_del(&(pos_p->list));
	}

	/* free the models list */
	list_for_each_entry_safe(pos_m, n_m, &f_priv->models, list) {
		list_del(&(pos_m->list));
	}

	free(f);
}
