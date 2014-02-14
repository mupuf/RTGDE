#include "pred_packets.h"
#include "prediction.h"
#include <string.h>
#include <math.h>

typedef struct {
	sample_time_t prediction_length;
	enum pred_avr_confidence_t confidence_factor;
} pred_packets_t;


pred_packets_t * pred_packets_priv(prediction_t* p)
{
	return (pred_packets_t*) p->user;;
}

int pred_packets_check(prediction_t *p)
{
	return 0;
}

prediction_list_t *pred_packets_exec(prediction_t *p)
{
	pred_packets_t *packets_priv = pred_packets_priv(p);
	uint64_t sum_size = 0, sum_size_sq = 0, count = 0;
	sample_value_t p_high, p_average, p_low, p_count;
	float avr_size, avr_sq_size, std_size;
	metric_t *packets_m;
	uint64_t time_span;
	int i;

	packets_m = prediction_find_metric(p, "packets");

	if (metric_is_empty(packets_m))
		return NULL;

	prediction_list_t *pl = prediction_list_create();
	if (!pl)
		return NULL;

	prediction_metric_result_t *r_size, *r_count;
	r_size = prediction_metric_result_create("packetSize", scoring_normal);
	r_count = prediction_metric_result_create("packetCount", scoring_normal);
	if (!r_size || !r_count) {
		if (r_size)
			prediction_metric_result_delete(r_size);
		if (r_count)
			prediction_metric_result_delete(r_count);
		return pl;
	}

	/* read the history of the metric */
	r_size->metric = packets_m;
	r_size->hsize = metric_history_size(packets_m);
	r_size->history = calloc(r_size->hsize, sizeof(sample_t));
	r_size->hsize = metric_dump_history(packets_m, r_size->history, r_size->hsize);

	/*r_count->hsize = r_size->hsize;
	r_count->history = calloc(r_count->hsize, sizeof(sample_t));
	memcpy(r_count->history, r_size->history, r_count->hsize * sizeof(sample_t));*/


	/* compute the average and variance of the size + wake up counts */
	time_span = r_size->history[r_size->hsize - 1].time - r_size->history[0].time;
	time_span = time_span > 0 ? time_span : 1;
	for (i = 0; i < r_size->hsize; i++) {
		sum_size += r_size->history[i].value;
		sum_size_sq += r_size->history[i].value * r_size->history[i].value;
		count++;
	}
	avr_size = ((float)sum_size) / r_size->hsize;
	avr_sq_size = ((float)sum_size_sq) / r_size->hsize;
	std_size = sqrtf(avr_sq_size - (avr_size * avr_size));
	p_high = avr_size + packets_priv->confidence_factor * std_size;
	p_average = avr_size;
	p_low = avr_size - packets_priv->confidence_factor * std_size;
	p_count = count * packets_priv->prediction_length / time_span;

	/* size metric */
	graph_add_point((graph_t *)r_size->high, 0, p_high);
	graph_add_point((graph_t *)r_size->high,
			packets_priv->prediction_length,
			p_high);

	graph_add_point((graph_t *)r_size->average, 0, p_average);
	graph_add_point((graph_t *)r_size->average,
			packets_priv->prediction_length,
			p_average);

	graph_add_point((graph_t *)r_size->low, 0, p_low);
	graph_add_point((graph_t *)r_size->low,
			packets_priv->prediction_length,
			p_low);

	/* count metric */
	graph_add_point((graph_t *)r_count->high, 0, p_count);
	graph_add_point((graph_t *)r_count->high,
			packets_priv->prediction_length,
			p_count);

	graph_add_point((graph_t *)r_count->average, 0, p_count);
	graph_add_point((graph_t *)r_count->average,
			packets_priv->prediction_length,
			p_count);

	graph_add_point((graph_t *)r_count->low, 0, p_count);
	graph_add_point((graph_t *)r_count->low,
			packets_priv->prediction_length,
			p_count);

	prediction_list_append(pl, r_size);
	prediction_list_append(pl, r_count);

	return pl;
}

void pred_packets_dtor(prediction_t *p)
{
	free(p->user);
}

prediction_t * pred_packets_create(sample_time_t prediction_length,
				   enum pred_avr_confidence_t confidence_factor)
{
	pred_packets_t *packets_priv = malloc(sizeof(pred_packets_t));
	if (!packets_priv)
		return NULL;

	packets_priv->prediction_length = prediction_length;
	packets_priv->confidence_factor = confidence_factor;
	return prediction_create(pred_packets_check,
				 pred_packets_exec,
				 pred_packets_dtor,
				 "predPackets",
				 (void *)packets_priv);
}

