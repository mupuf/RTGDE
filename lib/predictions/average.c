#include "prediction.h"
#include "../prediction_priv.h"
#include "predictions/average.h"
#include <stdint.h>
#include <math.h>

typedef struct {
	sample_time_t prediction_length;
	enum pred_avr_confidence_t confidence_factor;
} prediction_average_t;

prediction_average_t * prediction_average(prediction_t* p)
{
	return (prediction_average_t*) p->user;
}

int prediction_average_check(prediction_t *p)
{
	return 0;
}

prediction_list_t *prediction_average_exec(prediction_t *p)
{
	prediction_average_t *average = prediction_average(p);
	prediction_priv_t *p_priv = prediction_priv(p);
	prediction_metric_t *pos;

	prediction_list_t *pl = prediction_list_create();
	if (!pl)
		return NULL;

	/* for all input metrics */
	list_for_each_entry(pos, &p_priv->metrics, list) {
		sample_value_t p_high, p_average, p_low;
		uint64_t sum = 0, sum_sq = 0;
		float avr, avr_sq, std;
		int i;

		if (metric_is_empty(pos->base))
			continue;

		prediction_metric_result_t *r;
		r = prediction_metric_result_create(metric_name(pos->base));

		/* read the history of the metric */
		r->hsize = metric_history_size(pos->base);
		r->history = calloc(r->hsize, sizeof(sample_t));
		r->hsize = metric_dump_history(pos->base, r->history, r->hsize);

		/* compute the average and variance */
		for (i = 0; i < r->hsize; i++) {
			sum += r->history[i].value;
			sum_sq += r->history[i].value * r->history[i].value;
		}
		avr = ((float)sum) / r->hsize;
		avr_sq = ((float)sum_sq) / r->hsize;
		std = sqrtf(avr_sq - (avr * avr));

		/* compute the high, average and low points */
		p_high = avr + average->confidence_factor * std;
		p_average = avr;
		p_low = avr - average->confidence_factor * std;


		/* Add the points to the prediction */
		graph_add_point((graph_t *)r->high, 0, p_high);
		graph_add_point((graph_t *)r->high,
				average->prediction_length,
				p_high);

		graph_add_point((graph_t *)r->average, 0, p_average);
		graph_add_point((graph_t *)r->average,
				average->prediction_length,
				p_average);

		graph_add_point((graph_t *)r->low, 0, p_low);
		graph_add_point((graph_t *)r->low,
				average->prediction_length,
				p_low);

		prediction_list_append(pl, r);
	}

	return pl;
}

void prediction_average_dtor(prediction_t *p)
{
	free(p->user);
}

prediction_t * prediction_average_create(sample_time_t prediction_length,
				enum pred_avr_confidence_t confidence_factor)
{
	prediction_average_t *average = malloc(sizeof(prediction_average_t));
	if (!average)
		return NULL;

	average->prediction_length = prediction_length;
	average->confidence_factor = confidence_factor;
	return prediction_create(prediction_average_check,
				 prediction_average_exec,
				 prediction_average_dtor,
				 "pred_average",
				 (void *)average);
}
