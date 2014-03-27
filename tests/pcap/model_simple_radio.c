/*Copyright (c) 2014 Martin Peres
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.

 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#include "model_simple_radio.h"
#include "model.h"
#include <assert.h>
#include <inttypes.h>

typedef struct {
	uint32_t bitrate;
	float energy_rx_to_tx;
	float energy_tx_to_rx;
	uint32_t delay_rx_to_tx_us;
	uint32_t delay_tx_to_rx_us;
	float pwr_rx_idle;
	float pwr_tx;
} model_simple_radio_t;


model_simple_radio_t * model_simple_radio(model_t* m)
{
	return (model_simple_radio_t*) model_user(m);
}

decision_input_model_t * model_simple_radio_exec(model_t *m, prediction_list_t *input)
{
	model_simple_radio_t *msr = model_simple_radio(m);
	prediction_metric_result_t *psize, *pcount, *ppower, *pocc, *plat;
	const sample_t *packet_count, *s_size, *s_size_next;
	decision_input_metric_t *di_metric_occupancy, *di_metric_latency, *di_metric_pwr;
	decision_input_model_t *dim;

	psize = prediction_list_find(input, "packetSize");
	if (!psize) {
		fprintf(stderr, "model_simple_radio: metric 'packetSize'' has not been found!\n");
		return NULL;
	}

	pcount = prediction_list_find(input, "packetCount");
	if (!pcount) {
		fprintf(stderr, "model_simple_radio: metric 'packetCount'' has not been found!\n");
		return NULL;
	}

	ppower = prediction_list_find(input, "power");
	if (!ppower) {
		fprintf(stderr, "model_simple_radio: metric 'power' constraint has not been found!\n");
		return NULL;
	}

	pocc = prediction_list_find(input, "RF-occupancy");
	if (!pocc) {
		fprintf(stderr, "model_simple_radio: metric 'RF-occupancy' constraint has not been found!\n");
		return NULL;
	}

	plat = prediction_list_find(input, "nif-latency");
	if (!plat) {
		fprintf(stderr, "model_simple_radio: metric 'nif_latency'' constraint has not been found!\n");
		return NULL;
	}

	dim = decision_input_model_create(m);
	if (!dim)
		 return NULL;

	graph_t * o_rf_occupancy = graph_create();
	graph_t * o_card_latency = graph_create();
	graph_t * o_pwr = graph_create();
	assert(o_rf_occupancy);
	assert(o_card_latency);
	assert(o_pwr);

	packet_count = graph_read_last(pcount->average);

	s_size = graph_read_first(psize->average);
	s_size_next = graph_read_next(psize->average, s_size);
	while (s_size && s_size_next) {
		int64_t time_diff = s_size_next->time - s_size->time;
		uint64_t packet_count_interval = packet_count->value * time_diff / packet_count->time;
		int64_t rx_time = time_diff, tx_time, latency = 0;
		sample_value_t rf_occupancy, card_latency, pwr;
		float total_energy = 0.0;

		packet_count_interval = packet_count_interval > 0?packet_count_interval:1;

		tx_time = packet_count_interval * s_size->value * 8000000 / msr->bitrate;
		rx_time -= packet_count_interval * msr->delay_rx_to_tx_us;
		rx_time -= packet_count_interval * msr->delay_tx_to_rx_us;
		rx_time -= tx_time;

		if (rx_time < 0)
			latency = -rx_time;
		else
			latency = 0;

		total_energy += (float)packet_count_interval * msr->energy_rx_to_tx;
		total_energy += (float)packet_count_interval * msr->energy_tx_to_rx;
		total_energy += rx_time * msr->pwr_rx_idle / 1000000;
		total_energy += tx_time * msr->pwr_tx / 1000000;

		rf_occupancy = tx_time * 100.0 / time_diff;
		card_latency = latency / packet_count_interval;
		pwr = total_energy * 1000 * 1000000.0 / time_diff;

		rf_occupancy = rf_occupancy < 100?rf_occupancy:100;

		graph_add_point(o_rf_occupancy, s_size->time, rf_occupancy);
		graph_add_point(o_card_latency, s_size->time, card_latency);
		graph_add_point(o_pwr, s_size->time, pwr);

		s_size = s_size_next;
		s_size_next = graph_read_next(psize->average, s_size);

		/* this is the end of the dataset, add the final point with the previous data */
		if (!s_size_next) {
			graph_add_point(o_rf_occupancy, s_size->time, rf_occupancy);
			graph_add_point(o_card_latency, s_size->time, card_latency);
			graph_add_point(o_pwr, s_size->time, pwr);
		}
	}

	di_metric_occupancy = decision_input_metric_create("RF occupency",
							   prediction_metric_result_copy(pocc),
							   o_rf_occupancy);
	di_metric_latency = decision_input_metric_create("Emission latency",
							 prediction_metric_result_copy(plat),
							 o_card_latency);
	di_metric_pwr = decision_input_metric_create("Power consumption",
						     prediction_metric_result_copy(ppower),
						     o_pwr);
	decision_input_model_add_metric(dim, di_metric_occupancy);
	decision_input_model_add_metric(dim, di_metric_latency);
	decision_input_model_add_metric(dim, di_metric_pwr);

	return dim;
}

void model_simple_radio_delete(model_t *m)
{
	free(model_simple_radio(m));
}

model_t * model_simple_radio_create(const char *name, uint32_t bitrate,
				    float energy_rx_to_tx,
				    float energy_tx_to_rx,
				    uint32_t delay_rx_to_tx_us,
				    float delay_tx_to_rx_us,
				    float pwr_rx_idle, float pwr_tx)
{
	model_simple_radio_t *radio_simple = malloc(sizeof(model_simple_radio_t));
	if (!radio_simple)
		return NULL;

	radio_simple->bitrate = bitrate;
	radio_simple->delay_rx_to_tx_us = delay_rx_to_tx_us;
	radio_simple->delay_tx_to_rx_us = delay_tx_to_rx_us;
	radio_simple->energy_rx_to_tx = energy_rx_to_tx;
	radio_simple->energy_tx_to_rx = energy_tx_to_rx;
	radio_simple->pwr_rx_idle = pwr_rx_idle;
	radio_simple->pwr_tx = pwr_tx;

	return model_create(model_simple_radio_exec, model_simple_radio_delete,
			    name, (void *)radio_simple);
}

