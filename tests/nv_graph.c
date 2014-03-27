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

#include <stdio.h>
#include <assert.h>
#include <rtgde.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>

#include <predictions/pred_fsm.h>
#include <models/dummy.h>

struct pcap_packet {
	uint64_t timestamp;
	uint32_t len;
};

void die(const char *str)
{
	perror(str);
	exit(1);
}

void read_file(metric_t * me, FILE *finput)
{
	size_t len = 0;
	char *line = NULL;
	uint64_t time, time_diff;
	uint32_t val, last_val = -1;
	uint64_t count = 0;

	while (getline(&line, &len, finput) != -1) {
		sscanf(line, "%"PRIu64"[+%"PRIu64"]: %x\n", &time, &time_diff, &val);

		val &= 1;
		time /= 1000;

		if (last_val == val) {
			continue;
		}

		//metric_update(me, time - 1, last_val);
		metric_update(me, time, val);

		last_val = val;
		count++;
	}

	free(line);
}

void usage(int argc, char **argv)
{
	fprintf(stderr, "Usage: %s [filepath]\n", argv[0]);
}

void do_work(int argc, char *argv[], flowgraph_t *f, metric_t * me)
{
	FILE *finput = stdin;

	if (argc == 2) {
		finput = fopen(argv[1], "rb");
		if (!finput)
			die("Cannot open the input file");
	}

	read_file(me, finput);
	rtgde_start(f, 1);
	flowgraph_teardown(f);
}

struct fsm_pred_throuput_state {
	fsm_state_t *fsm_st;
	int power;
};

struct fsm_pred_throuput {
	struct fsm_pred_throuput_state on;
	struct fsm_pred_throuput_state off;
	fsm_state_t *cur;
} fsm_pred_data;

fsm_state_t *fsm_pred_throuput_next_state(fsm_t *fsm, const char *metric, sample_value_t value)
{
	if (value & 1)
		return fsm_pred_data.off.fsm_st;
	else
		return fsm_pred_data.on.fsm_st;
}

int fsm_pred_throuput_metric_from_state(fsm_state_t *state,
					const char *metric_name,
					sample_value_t *value)
{
	struct fsm_pred_throuput_state *st = fsm_state_get_user(state);
	if (strcmp(metric_name, "pgraph usage") == 0) {
		*value = st->power;
		return 0;
	} else
		return 1;
}

int main(int argc, char *argv[])
{
	fsm_t *pred_fsm = fsm_create(fsm_pred_throuput_next_state, NULL, &fsm_pred_data);
	fsm_pred_data.on.fsm_st = fsm_add_state(pred_fsm, "ACTIVE", &fsm_pred_data.on);
	fsm_pred_data.off.fsm_st = fsm_add_state(pred_fsm, "IDLE", &fsm_pred_data.off);
	fsm_pred_data.cur = fsm_pred_data.off.fsm_st;

	prediction_t * mp = prediction_fsm_create(pred_fsm,
						  fsm_pred_throuput_metric_from_state,
						  100000, 10);
	assert(mp);
	prediction_fsm_dump_probability_density(mp, "fsm_pred_pgraph");

	metric_t * me = metric_create("pgraph usage", "on or off", 1000000);
	assert(me);

	assert(!prediction_attach_metric(mp, me));
	flowgraph_t *f = flowgraph_create("perflvl_decision", NULL, NULL,
					  NULL, NULL, 1000000);
	assert(!flowgraph_attach_prediction(f, mp));

	model_t * m = model_dummy_create("dummy");
	assert(m);

	assert(!flowgraph_attach_model(f, m));

	flowgraph_model_output_csv(f, "pred_graph_idle_%s_%s_%i.csv", NULL);

	do_work(argc, argv, f, me);

	model_delete(m);

	prediction_delete(mp);

	metric_delete(me);

	return 0;
}
