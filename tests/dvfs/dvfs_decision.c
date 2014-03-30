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

#include "dvfs_decision.h"
#include <assert.h>
#include "utils.h"

struct ring {
	size_t size;
	size_t capacity;
	float *data;
};

enum core {
	NONE = 0,
	BIG = 1,
	LITTLE = 2
};

typedef struct {
	struct ring sum_usage;

	int64_t last_update;
	enum core cur_core;
} decision_dvfs_t;

void ring_init(struct ring *ring, size_t capacity)
{
	ring->size = 0;
	ring->capacity = capacity;
	ring->data = malloc(capacity * sizeof(float));
}

void ring_free_data(struct ring *ring)
{
	free(ring->data);
}

void ring_add(struct ring *ring, float data)
{
	ring->data[ring->size % ring->capacity] = data;
	ring->size++;
}

float ring_avr(struct ring *ring)
{
	size_t count, i;
	float sum = 0.0;

	count = ring->size > ring->capacity ? ring->capacity : ring->size;

	for (i = 0; i < count; i++)
		sum += ring->data[i];

	return sum / count;
}

decision_input_model_t *decision_dvfs_calc(decision_t *d, decision_input_t *di)
{
	decision_dvfs_t *pcap = (decision_dvfs_t*)decision_user(d);
	decision_input_model_t *big, *little;
	decision_input_metric_t *cpu_usage;
	enum core choice = NONE;
	int64_t time_since_last_sw;

	big = decision_input_model_get_by_name(di, "core-BIG");
	little = decision_input_model_get_by_name(di, "core-LITTLE");

	if (!big || !little)
		return NULL;

	cpu_usage = decision_input_metric_from_name(little, "CPU_usage");
	if (!cpu_usage)
		return NULL;
#if 0
	/*ring_add(&pcap->sum_gsm_score_diff, gsm->score - wifi->score);
	ring_add(&pcap->sum_gsm_latency, gsm_latency->score);

	float score_diff_avr = ring_avr(&pcap->sum_gsm_score_diff);
	float gsm_lat_avr = ring_avr(&pcap->sum_gsm_latency);

	if (pcap->cur_if == NONE) {
		if (score_diff_avr > 0) {
			choice = GSM;
			goto out;
		} else {
			choice = WIFI;
			goto out;
		}
	}

	if (score_diff_avr > 0 && gsm_lat_avr > 0.9)
		choice = GSM;
	else
		choice = WIFI;

out:
	time_since_last_sw = clock_read_us() - pcap->last_update;
	if (choice > NONE && time_since_last_sw >= 10000000) {
		pcap->cur_if = choice;
		pcap->last_update = clock_read_us();
		return choice == GSM ? gsm : wifi;
	} else {
		/* return the current one */
		return pcap->cur_if == GSM ? gsm : wifi;
	}
#endif

	return big;
}

void decision_dvfs_dtor(decision_t *d)
{
	decision_dvfs_t *dvfs = (decision_dvfs_t*)decision_user(d);

	ring_free_data(&dvfs->sum_usage);
	free(decision_user(d));
}

decision_t * decision_dvfs_create()
{
	decision_dvfs_t *dvfs = malloc(sizeof(decision_dvfs_t));
	if (!dvfs)
		return NULL;

	ring_init(&dvfs->sum_usage, 30);

	dvfs->last_update = 0;
	dvfs->cur_core = NONE;

	return decision_create(decision_dvfs_calc, decision_dvfs_dtor,
			       "decision_dvfs", (void *)dvfs);
}
