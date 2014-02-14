#include <inttypes.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <time.h>
#include <assert.h>

#include "rtgde.h"
#include "list.h"

#define MIN(a,b) (((a)<(b))?(a):(b))
#define MAX(a,b) (((a)>(b))?(a):(b))

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
	int one_time;

	flowgraph_callback_t user_cb;
	void *user_cb_data;

	/* CSV output */
	char * csv_pred_format, *csv_model_format;
	flowgraph_prediction_output_csv_cb_t csv_pred_cb;
	flowgraph_model_output_csv_cb_t csv_model_cb;
	uint64_t flowgraph_exec_count;
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

static void csv_log_pred(flowgraph_priv_t *f_priv)
{
	prediction_metric_result_t *pmr;
	flowgraph_prediction_t *pos_p;

	if (!f_priv->csv_pred_format)
		return;

	list_for_each_entry(pos_p, &f_priv->predictions, list) {

		pmr = prediction_list_get_first(pos_p->last_prediction);
		while (pmr) {
			int64_t last_sample_time = 0;
			char filename[PATH_MAX];
			int have_metric = pmr->metric && pmr->hsize > 0;
			int i;

			snprintf(filename, sizeof(filename), f_priv->csv_pred_format,
				 prediction_name(pos_p->base), pmr->name,
				 f_priv->flowgraph_exec_count);

			FILE *f = fopen(filename, "w");
			if (!f) {
				perror("rtgde csv output, fopen");
				continue;
			}

			if (have_metric) {
				fprintf(f, "Time (µs), %s, %s prediction low, "
					"%s prediction average, %s prediction high\n",
					metric_name(pmr->metric), pmr->name, pmr->name,
					pmr->name);

				/* dump the history */
				last_sample_time = pmr->history[pmr->hsize - 1].time;
				for (i = 0; i < pmr->hsize; i++) {
					if (i > 0)
						fprintf(f, "%" PRIi64 ", %i, , , ,\n",
							pmr->history[i].time - 1 - last_sample_time,
							pmr->history[i - 1].value);
					fprintf(f, "%" PRIi64 ", %i, , , ,\n",
						pmr->history[i].time - last_sample_time,
						pmr->history[i].value);
				}
				last_sample_time++;
			} else {
				fprintf(f, "Time (µs), %s %s-low, "
					"%s %s-average, %s %s-high\n",
					pmr->name, pmr_usage_hint_to_str(pmr->usage_hint),
					pmr->name, pmr_usage_hint_to_str(pmr->usage_hint),
					pmr->name, pmr_usage_hint_to_str(pmr->usage_hint));

				last_sample_time = 0;
			}

			/* dump the predicted values + model output */
			const sample_t *s_low = graph_read_first(pmr->low);
			const sample_t *s_avg = graph_read_first(pmr->average);
			const sample_t *s_high = graph_read_first(pmr->high);

			assert(s_low->time == 0);
			assert(s_avg->time == 0);
			assert(s_high->time == 0);

			while (s_low && s_avg && s_high) {
				fprintf(f, "%" PRIu64 ", %s%i, %i, %i\n",
					s_low->time, have_metric?", ":"",
					s_low->value, s_avg->value, s_high->value);

				s_low = graph_read_next(pmr->low, s_low);
				s_avg = graph_read_next(pmr->average, s_avg);
				s_high = graph_read_next(pmr->high, s_high);
			}

			fclose(f);

			if (f_priv->csv_pred_cb)
				f_priv->csv_pred_cb((flowgraph_t *)f_priv,
						    pos_p->base, pmr, filename);
			pmr = prediction_list_get_next(pos_p->last_prediction, pmr);
		}
	}
}

static void csv_log_models(flowgraph_priv_t *f_priv, decision_input_t *di)
{
	decision_input_model_t *dim;
	decision_input_metric_t* m;

	if (!f_priv->csv_model_format)
		return;

	dim = decision_input_model_get_first(di);
	while (dim) {
		m = decision_input_metric_get_first(dim);
		while (m) {
			int64_t last_sample_time = 0;
			char filename[PATH_MAX];
			int i;

			snprintf(filename, sizeof(filename), f_priv->csv_model_format,
				 model_name(dim->model), m->name,
				 f_priv->flowgraph_exec_count);

			FILE *f = fopen(filename, "w");
			if (!f) {
				perror("rtgde csv output, fopen");
				continue;
			}

			fprintf(f, "Time, %s, %s prediction low, %s prediction average, "
				"%s prediction high, model '%s' %s (score = %.3f)\n",
				m->prediction->name,
				m->prediction->name,
				m->prediction->name,
				m->prediction->name,
				model_name(m->parent->model), m->name,
				m->score);

			/* dump the history */
			if (m->prediction->hsize > 0) {
				last_sample_time = m->prediction->history[m->prediction->hsize - 1].time;
				for (i = 0; i < m->prediction->hsize; i++) {
					if (i > 0)
						fprintf(f, "%" PRIi64 ", %i, , , ,\n",
							m->prediction->history[i].time - 1 - last_sample_time,
							m->prediction->history[i - 1].value);
					fprintf(f, "%" PRIi64 ", %i, , , ,\n",
						m->prediction->history[i].time - last_sample_time,
						m->prediction->history[i].value);
				}
				last_sample_time++;
			} else
				last_sample_time = 0;

			/* dump the predicted values + model output */
			const sample_t *s_low = graph_read_first(m->prediction->low);
			const sample_t *s_avg = graph_read_first(m->prediction->average);
			const sample_t *s_high = graph_read_first(m->prediction->high);
			const sample_t *s_model = graph_read_first(m->output);

			assert(s_low->time == 0);
			assert(s_avg->time == 0);
			assert(s_high->time == 0);
			assert(s_model->time == 0);

			const sample_t *s_n_pred = s_low, *s_n_mod = s_model;
			while (s_low && s_avg && s_high && s_model &&
			       !(s_n_pred == NULL && s_n_mod == NULL)) {
				sample_time_t max_time = MAX(s_low->time, s_model->time);

				fprintf(f, "%" PRIu64 ", , %i, %i, %i, %i\n",
					max_time, s_low->value,
					s_avg->value, s_high->value, s_model->value);

				s_n_pred = graph_read_next(m->prediction->low, s_low);
				s_n_mod = graph_read_next(m->output, s_model);

				/*sample_time_t time_next_sample;
				if (s_n_pred && s_n_mod)
					time_next_sample = MIN(s_n_pred->time, s_n_mod->time);
				else if (s_n_pred)
					time_next_sample = s_n_pred->time;
				else if (s_n_mod)
					time_next_sample = s_n_mod->time;
				else
					continue;

				fprintf(f, "%" PRIu64 ", , %i, %i, %i, %i\n",
					time_next_sample - 1,
					s_low->value, s_avg->value, s_high->value,
					s_model->value);*/

				if ((!s_n_mod && s_n_pred) ||
				    (s_n_pred && s_n_pred->time <= s_n_mod->time)) {
					s_low = s_n_pred;
					s_avg = graph_read_next(m->prediction->average, s_avg);
					s_high = graph_read_next(m->prediction->high, s_high);
					assert(s_low->time == s_avg->time);
					assert(s_avg->time == s_high->time);
				}
				if ((!s_n_pred && s_n_mod) ||
				    (s_n_mod && s_n_pred->time >= s_n_mod->time))
					s_model = s_n_mod;
			}

			fclose(f);

			if (f_priv->csv_model_cb)
				f_priv->csv_model_cb((flowgraph_t *)f_priv, m, filename);

			m = decision_input_metric_get_next(m);
		}

		dim = decision_input_model_get_next(dim);
	}
}

static void execute_flow_graph(flowgraph_priv_t *f_priv)
{
	flowgraph_prediction_t *pos_p;
	flowgraph_model_t *pos_m;
	decision_input_model_t *dim;
	decision_input_t *di = decision_input_create();
	decision_input_model_t *dim_sel = NULL;

	pthread_mutex_lock(&f_priv->config_mutex);

	/* exec all attached predictions */
	list_for_each_entry(pos_p, &f_priv->predictions, list) {
		pos_p->last_prediction = prediction_exec(pos_p->base);
	}

	/* exec all the models */
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

	/* write a nice report to the disc */
	csv_log_pred(f_priv);
	csv_log_models(f_priv, di);

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

		f_priv->flowgraph_exec_count++;

		/* re-enable cancelation */
		s = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);
		if (s != 0)
			die(s, "pthread_setcancelstate");

		/* schedule the next work, if we weren't run as one_time */
		if (f_priv->one_time)
			return NULL;

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

	f_priv->csv_model_format = NULL;
	f_priv->flowgraph_exec_count = 0;

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

	/* look for this entry in the predictions list */
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

void flowgraph_prediction_output_csv(flowgraph_t *f, const char *csv_filename_format,
			  flowgraph_prediction_output_csv_cb_t output_cb)
{
	flowgraph_priv_t *f_priv = flowgraph_priv(f);

	if (f_priv->csv_pred_cb)
		free(f_priv->csv_pred_cb);
	if (f_priv->csv_pred_format)
		free(f_priv->csv_pred_format);

	f_priv->csv_pred_cb = output_cb;
	f_priv->csv_pred_format = strdup(csv_filename_format);
}

void flowgraph_model_output_csv(flowgraph_t *f, const char *csv_filename_format,
			  flowgraph_model_output_csv_cb_t output_cb)
{
	flowgraph_priv_t *f_priv = flowgraph_priv(f);

	if (f_priv->csv_model_format)
		free(f_priv->csv_model_format);
	if (f_priv->csv_model_format)
		free(f_priv->csv_model_format);

	f_priv->csv_model_cb = output_cb;
	f_priv->csv_model_format = strdup(csv_filename_format);
}

int rtgde_start(flowgraph_t *f, int one_time)
{
	flowgraph_priv_t *f_priv = flowgraph_priv(f);
	f_priv->one_time = one_time;
	int s = pthread_create(&f_priv->thread, NULL, &thread_flowgraph, f);

	if (s != 0 && one_time) {
		s = pthread_join(f_priv->thread, NULL);
	}

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

	free(f_priv->name);
	if (f_priv->csv_model_format)
		free(f_priv->csv_model_format);
	free(f);

	/* never free the mutex to spot the use-after-free in some cases */
}
