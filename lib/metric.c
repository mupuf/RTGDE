#include "metric_priv.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

metric_priv_t * metric_priv(metric_t* m)
{
	return (metric_priv_t *)m;
}

metric_t * metric_create(const char *name, history_size_t history_size)
{
	metric_priv_t *m_priv = malloc(sizeof(metric_priv_t));

	if (!m_priv)
		return NULL;

	m_priv->name = strdup(name);

	/* history */
	pthread_mutex_init(&m_priv->history_mutex, NULL);
	m_priv->history_size = history_size;
	m_priv->ring = malloc(history_size * sizeof(sample_t));
	m_priv->put = 0;
	m_priv->get = 0;
	m_priv->csv_output = NULL;

	return (metric_t *)m_priv;
}

const char *metric_name(metric_t* m)
{
	metric_priv_t *m_priv = metric_priv(m);
	return m_priv->name;
}

static history_size_t rb_next_index(history_size_t index, history_size_t history_size)
{
	return (index + 1) % history_size;
}

static history_size_t rb_previous_index(history_size_t index, history_size_t history_size)
{
	return (history_size + index - 1) % history_size;
}

void metric_print_history(metric_t *m)
{
	metric_priv_t *m_priv = metric_priv(m);
	history_size_t get;

	pthread_mutex_lock(&m_priv->history_mutex);

	get = m_priv->get;

	while (get != m_priv->put) {
		printf("[%llu, %i] ", (unsigned long long)m_priv->ring[get].time,
				      m_priv->ring[get].value);

		get = rb_next_index(get, m_priv->history_size);
	}
	printf("\n");

	pthread_mutex_unlock(&m_priv->history_mutex);
}

void metric_update(metric_t *m, sample_time_t timestamp, sample_value_t value)
{
	metric_priv_t *m_priv = metric_priv(m);

	pthread_mutex_lock(&m_priv->history_mutex);

	if (rb_next_index(m_priv->put, m_priv->history_size) == m_priv->get)
		m_priv->get = rb_next_index(m_priv->get, m_priv->history_size);

	m_priv->ring[m_priv->put].time = timestamp;
	m_priv->ring[m_priv->put].value = value;

	m_priv->put = rb_next_index(m_priv->put, m_priv->history_size);

	/* CSV output */
	if (m_priv->csv_output)
		fprintf(m_priv->csv_output, "%" PRIu64 ", %i\n", timestamp, value);

	pthread_mutex_unlock(&m_priv->history_mutex);
}

history_size_t metric_dump_history(metric_t *m, sample_t *buffer, history_size_t size)
{
	metric_priv_t *m_priv = metric_priv(m);
	history_size_t get;
	size_t i = 0;

	pthread_mutex_lock(&m_priv->history_mutex);

	get = m_priv->get;

	while (get != m_priv->put && i < size) {
		buffer[i].time = m_priv->ring[get].time;
		buffer[i].value = m_priv->ring[get].value;

		get = rb_next_index(get, m_priv->history_size);
		i++;
	}

	pthread_mutex_unlock(&m_priv->history_mutex);

	return i;
}

sample_t metric_get_last(metric_t *m)
{
	metric_priv_t *m_priv = metric_priv(m);
	history_size_t last;
	sample_t s;

	pthread_mutex_lock(&m_priv->history_mutex);
	last = rb_previous_index(m_priv->put, m_priv->history_size);

	s.time = m_priv->ring[last].time;
	s.value = m_priv->ring[last].value;
	pthread_mutex_unlock(&m_priv->history_mutex);

	return s;
}

int metric_is_empty(metric_t *m)
{
	metric_priv_t *m_priv = metric_priv(m);
	int empty;

	pthread_mutex_lock(&m_priv->history_mutex);
	empty = (m_priv->put == m_priv->get && m_priv->put == 0);
	pthread_mutex_unlock(&m_priv->history_mutex);

	return empty;
}

history_size_t metric_history_size(metric_t *m)
{
	metric_priv_t *m_priv = metric_priv(m);
	return m_priv->history_size;
}

void metric_set_csv_output_file(metric_t *m, const char *time_unit, const char *value_unit, FILE *of)
{
	metric_priv_t *m_priv = metric_priv(m);
	m_priv->csv_output = of;
	fprintf(of, "Time (%s), %s (%s)\n", time_unit, m_priv->name, value_unit);
}

void metric_delete(metric_t *m)
{
	metric_priv_t *m_priv = metric_priv(m);
	free((char *)m_priv->name);
	free(m_priv->ring);
	free(m_priv);
}
