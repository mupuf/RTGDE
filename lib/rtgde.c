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
	prediction_list_t *last_prediction;

	/* private declarations */
	struct list_head list;
} flowgraph_prediction_t;

typedef struct {
	model_t *base;

	/* private declarations */
	struct list_head list;
} flowgraph_model_t;

typedef struct {
	flowgraph_t base;

	/* configuration */
	pthread_mutex_t config_mutex;

	/* private declarations */
	char *name;
	struct list_head predictions;
	struct list_head models;
	scoring_t *scoring;
	decision_t *decision;
	pthread_t thread;
	uint64_t update_period_us;

	flowgraph_callback_t user_cb;
	void *user_cb_data;
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

static void execute_flow_graph(flowgraph_priv_t *f_priv)
{
	flowgraph_prediction_t *pos_p;
	flowgraph_model_t *pos_m;
	decision_input_model_t *dim;
	decision_input_t *di = decision_input_create();
	decision_input_model_t *dim_sel = NULL;

	pthread_mutex_lock(&f_priv->config_mutex);

	/* do the predictions for all attached predictions */
	list_for_each_entry(pos_p, &f_priv->predictions, list) {
		pos_p->last_prediction = prediction_exec(pos_p->base);
	}

	/* feed the predictions to the models */
	list_for_each_entry(pos_m, &f_priv->models, list) {
		prediction_list_t *predictions = prediction_list_create();

		/* unite all the predictions in one list */
		list_for_each_entry(pos_p, &f_priv->predictions, list) {
			prediction_list_append_list_copy(predictions,
							   pos_p->last_prediction);
		}

		/* feed the ouput of prediction to each models */
		dim = model_exec(pos_m->base, predictions);

		decision_input_add_model(di, dim);

		/* predictions will be freed by model_output_delete() */
	}

	/* feed the output of each models to the scoring */
	if (f_priv->scoring)
		scoring_exec(f_priv->scoring, di);

	/* take a decision */
	if (f_priv->decision)
		dim_sel = decision_exec(f_priv->decision, di);

	/* call back the user (TODO: Put that in a thread ?)*/
	if (f_priv->user_cb)
		f_priv->user_cb((flowgraph_t*)f_priv, di, dim_sel, f_priv->user_cb_data);

	/* free all the ressources */
	decision_input_delete(di);
	list_for_each_entry(pos_p, &f_priv->predictions, list) {
		prediction_list_delete(pos_p->last_prediction);
		pos_p->last_prediction = NULL;
	}

	pthread_mutex_unlock(&f_priv->config_mutex);
}

static void *thread_flowgraph (void *p_data)
{
	flowgraph_priv_t *f_priv = (flowgraph_priv_t *)p_data;
	int s;

	/* Get the clock */
	int64_t next_wakeup = clock_read_us();

	while (1) {
		next_wakeup += f_priv->update_period_us;

		/* disable cancelation while we are taking a decision */
		s = pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
		if (s != 0)
			die(s, "pthread_setcancelstate");

		/* real work */
		execute_flow_graph(f_priv);

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

flowgraph_t *flowgraph_create(const char *name, scoring_t *s, decision_t *d,
			      flowgraph_callback_t user_cb, void *user_cb_data,
			      uint64_t update_period_ns)
{
	flowgraph_priv_t *f_priv = malloc(sizeof(flowgraph_priv_t));
	if (!f_priv)
		return NULL;

	pthread_mutex_init(&f_priv->config_mutex, NULL);

	INIT_LIST_HEAD(&f_priv->predictions);
	INIT_LIST_HEAD(&f_priv->models);
	f_priv->scoring = s;
	f_priv->decision = d;
	f_priv->update_period_us = update_period_ns;
	f_priv->user_cb = user_cb;
	f_priv->user_cb_data = user_cb_data;
	f_priv->name = strdup(name);

	return (flowgraph_t *)f_priv;
}

const char *flowgraph_name(flowgraph_t *f)
{
	return flowgraph_priv(f)->name;
}

int flowgraph_attach_prediction(flowgraph_t *f, prediction_t * p)
{
	flowgraph_priv_t *f_priv = flowgraph_priv(f);
	flowgraph_prediction_t *fp;
	int ret = 0;

	pthread_mutex_lock(&f_priv->config_mutex);

	fp = malloc(sizeof(flowgraph_prediction_t));
	if (!fp) {
		ret = -1;
		goto exit;
	}

	fp->base = p;
	fp->last_prediction = NULL;

	list_add_tail(&fp->list, &f_priv->predictions);

exit:
	pthread_mutex_unlock(&f_priv->config_mutex);
	return ret;
}

int flowgraph_detach_prediction(flowgraph_t *f, prediction_t * p)
{
	flowgraph_priv_t *f_priv = flowgraph_priv(f);
	flowgraph_prediction_t *pos, *n;
	int ret = 0;

	pthread_mutex_lock(&f_priv->config_mutex);

	/* free the metrics list */
	list_for_each_entry_safe(pos, n, &f_priv->predictions, list) {
		if (pos->base == p) {
			list_del(&(pos->list));
			free(pos);
			goto exit;
		}
	}
	ret = -1;

exit:
	pthread_mutex_unlock(&f_priv->config_mutex);
	return ret;
}

int flowgraph_attach_model(flowgraph_t *f, model_t * m)
{
	flowgraph_priv_t *f_priv = flowgraph_priv(f);
	int ret = 0;

	pthread_mutex_lock(&f_priv->config_mutex);

	flowgraph_model_t *fm = malloc(sizeof(flowgraph_model_t));
	if (!fm) {
		ret = -1;
		goto exit;
	}

	fm->base = m;

	list_add_tail(&fm->list, &f_priv->models);

exit:
	pthread_mutex_unlock(&f_priv->config_mutex);
	return ret;
}

int flowgraph_detach_model(flowgraph_t *f, model_t * m)
{
	flowgraph_priv_t *f_priv = flowgraph_priv(f);
	flowgraph_model_t *pos, *n;
	int ret = 0;

	pthread_mutex_lock(&f_priv->config_mutex);

	/* free the metrics list */
	list_for_each_entry_safe(pos, n, &f_priv->models, list) {
		if (pos->base == m) {
			list_del(&(pos->list));
			free(pos);
			goto exit;
		}
	}
	ret = -1;

exit:
	pthread_mutex_unlock(&f_priv->config_mutex);
	return ret;
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

	pthread_mutex_lock(&f_priv->config_mutex);

	free(f_priv->name);

	/* free the prediction list */
	list_for_each_entry_safe(pos_p, n_p, &f_priv->predictions, list) {
		list_del(&(pos_p->list));
		prediction_list_delete(pos_p->last_prediction);
		free(pos_p);
	}

	/* free the models list */
	list_for_each_entry_safe(pos_m, n_m, &f_priv->models, list) {
		list_del(&(pos_m->list));
		free(pos_m);
	}

	free(f);

	/* never free the mutex to spot the use-after-free in some cases */
}
