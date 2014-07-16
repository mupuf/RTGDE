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

#undef NDEBUG
#include <stdio.h>
#include <assert.h>
#include <rtgde.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <inttypes.h>
#include <signal.h>

#include <predictions/average.h>
#include <predictions/constraint.h>
#include <scoring/simple.h>
#include "dvfs_decision.h"
#include "model_perflvl.h"
#include "utils.h"

#include <glibtop.h>
#include <glibtop/cpu.h>

#define DECISION_LOG_FILE "dvfs_decision_log.csv"
#define PERFLVL_BIG_LOG_FILE "dvfs_big_perflvl.csv"
#define PERFLVL_LITTLE_LOG_FILE "dvfs_little_perflvl.csv"

volatile int request_quit = 0;

struct flowgraph_data {
	metric_t * me_usage;
	prediction_t * p_pwr;
	prediction_t * p_usage;
	model_t * m_big;
	model_t * m_little;
	scoring_t * scoring;
	decision_t * decision;
	flowgraph_t *f;

	FILE *log_decision;
	FILE *log_perflvl_big;
	FILE *log_perflvl_little;
} data;

int64_t relative_time_us()
{
	static int64_t boot_time = 0;
	if (boot_time == 0)
		boot_time = clock_read_us();
	return clock_read_us() - boot_time;
}

int64_t timeval_to_us(struct timeval tv)
{
	return tv.tv_sec * 1000000 + tv.tv_usec;
}

void sig_request_quit(int signal)
{
	request_quit = 1;
	fprintf(stderr, "preparing to quit!\n");
}

void monitor_cpu_usage(metric_t * me, int64_t timeout_us)
{
	glibtop_init();

	glibtop_cpu cpu, prev_cpu;

	rtgde_start(data.f, 0);

	glibtop_get_cpu (&prev_cpu);
	while (!request_quit) {
		usleep(100000);
		glibtop_get_cpu (&cpu);

		guint64 total_diff = cpu.total - prev_cpu.total;
		guint64 idle_diff = cpu.idle - prev_cpu.idle;
		int32_t usage = 100 - (idle_diff * 100 / total_diff);

		fprintf(stderr, "usage = %i\n", usage);

		metric_update(me, relative_time_us(), usage);

		prev_cpu = cpu;
	}
}

void read_from_file(metric_t * me, const char *filepath)
{
	uint64_t timestamp;
	int32_t usage;

	FILE *finput = fopen(filepath, "rb");
	if (!finput) {
		fprintf(stderr, "Cannot open the input file '%s'\n", filepath);
		exit(1);
	}

	rtgde_start(data.f, 0);
	while(fscanf(finput, "%" PRIu64 ", %i\n", &timestamp, &usage) == 2 && !request_quit) {
		int64_t sleep_time = timestamp - relative_time_us();
		if (sleep_time > 0)
			usleep(sleep_time);
		fprintf(stderr, "usage = %i\n", usage);
		metric_update(me, timestamp, usage);
	}
}

void usage(int argc, char **argv)
{
	fprintf(stderr, "Usage: %s filepath\n", argv[0]);
}

void do_work(int argc, char *argv[], metric_t * me, int64_t timeout_us)
{
	if (argc == 1) {
		monitor_cpu_usage(me, timeout_us);
		return;
	}

	if (argc == 2)
		read_from_file(me, argv[1]);
	else
		usage(argc, argv);
}

void flowgraph_prediction_output_csv_cb(flowgraph_t *f,
					  prediction_t *p,
					  prediction_metric_result_t *pmr,
					  const char *csv_filename)
{
	char cmd[1024];
	const char *gnuplot_file = "../gnuplot/dvfs/prediction.plot";
	const char *m_name = metric_name(pmr->metric), *m_unit = metric_unit(pmr->metric);

	if (!m_name)
		m_name = "No input metric associated";
	if (!m_unit)
		m_unit = "no unit";

	snprintf(cmd, sizeof(cmd),
		 "gnuplot -e \"filename='%s'\" -e \"graph_title='%s %s'\""
		 " -e \"metric='%s (%s)'\" -e \"prediction='predicted %s (%s)'\""
		 " %s 2> /dev/null > /dev/null",
		 csv_filename, pmr_usage_hint_to_str(pmr->usage_hint),
		 pmr->name, m_name, m_unit, pmr->name, pmr->unit, gnuplot_file);
	system(cmd);
}

void flowgraph_model_csv_cb(flowgraph_t *f, decision_input_metric_t* m,
					  const char *csv_filename)
{
	char cmd[1024];
	const char *gnuplot_file = "../gnuplot/dvfs/model_output_scored.plot";

	if (m->prediction->scoring_style == scoring_inverted) {
		gnuplot_file = "../gnuplot/model_output_scored_inverted.plot";
	}

	snprintf(cmd, sizeof(cmd),
		 "gnuplot -e \"filename='%s'\" -e \"graph_title='%s'\""
		 " -e \"prediction='%s (%s)'\" %s 2> /dev/null > /dev/null",
		 csv_filename, m->name, m->prediction->name,
		 m->prediction->unit, gnuplot_file);
	system(cmd);
}

void scoring_output_csv_cb(scoring_t *s, const char *name, const char *csv_filename)
{
	char cmd[1024];

	snprintf(cmd, sizeof(cmd), "gnuplot -e \"filename='%s'\" -e "
	       "\"graph_title='Score of the BIG and Little models for the %s metric.'\""
	       " ../gnuplot/dvfs/score_log.plot", csv_filename, name);
	system(cmd);
}

void generate_perflvl_changes(FILE *file, const model_t *m, const char *name, const char *csv_name, uint64_t timestamp)
{
	fprintf(file, "%" PRIu64 ", %zu\n", timestamp, model_core_current_perflvl(m));
	fflush(file);

	char cmd[1024];
	snprintf(cmd, sizeof(cmd), "gnuplot -e \"filename='%s'\" -e "
		 "\"graph_title='Evolution of the performance level of the %s model'\""
		 " ../gnuplot/dvfs/perflvl_log.plot", csv_name, name);
	system(cmd);
}

void decision_callback(flowgraph_t *f, decision_input_t *di,
		       decision_input_model_t *dim, void *user)
{
	//struct flowgraph_data *d = (struct flowgraph_data*) user;
	decision_input_model_t *big, *little;

	if (!dim) {
		fprintf(stderr, "Callback decision: no decision has been made!\n");
		return;
	}

	big = decision_input_model_get_by_name(di, "core-BIG");
	if (!big) {
		fprintf(stderr, "big model not found!\n");
		return;
	}

	little = decision_input_model_get_by_name(di, "core-LITTLE");
	if (!little) {
		fprintf(stderr, "little model not found!\n");
		return;
	}

	uint64_t timestamp = relative_time_us();
	fprintf(data.log_decision, "%" PRIu64 ", %f, %f, %f, %f, 0\n", timestamp,
		big->score, little->score,
		big==dim?big->score:0, little==dim?little->score:0);
	fflush(data.log_decision);
	fsync(fileno(data.log_decision));

	system("gnuplot -e \"filename='"DECISION_LOG_FILE"'\" -e "
	       "\"graph_title='Decision result for the BIG and LITTLE models'\""
	       " ../gnuplot/dvfs/decision_log.plot");

	generate_perflvl_changes(data.log_perflvl_big, big->model, "BIG", PERFLVL_BIG_LOG_FILE, timestamp);
	generate_perflvl_changes(data.log_perflvl_little, little->model, "LITTLE", PERFLVL_LITTLE_LOG_FILE, timestamp);
}

int main(int argc, char *argv[])
{
	data.log_decision = fopen(DECISION_LOG_FILE, "w");
	if (!data.log_decision)  {
		perror("cannot open '" DECISION_LOG_FILE "'");
		return 0;
	}
	fprintf(data.log_decision, "time (µs), BIG score, LITTLE score, BIG selected, LITTLE selected, always 0\n");

	data.log_perflvl_big = fopen(PERFLVL_BIG_LOG_FILE, "w");
	data.log_perflvl_little = fopen(PERFLVL_LITTLE_LOG_FILE, "w");

	signal(SIGINT, sig_request_quit);
	signal(SIGQUIT, sig_request_quit);
	signal(SIGABRT, sig_request_quit);
	signal(SIGTERM, sig_request_quit);

	data.p_usage = prediction_average_create(1000000, 2);
	assert(data.p_usage);

	data.me_usage = metric_create("CPU usage", "%", 10);
	assert(data.me_usage);

	assert(!prediction_attach_metric(data.p_usage, data.me_usage));

	data.p_pwr = prediction_constraint_create("power", "mW", 1000000,
						  5, 125, 1000, scoring_inverted);

	data.scoring = score_simple_create();
	assert(data.scoring);

	assert(scoring_metric_create(data.scoring, "Power consumption", 10));
	assert(scoring_metric_create(data.scoring, "CPU usage", 13));

	data.decision = decision_dvfs_create();
	assert(data.decision);

	model_core_set_perflvl_t perfs_big[] = { { 100, 800, 0.25 }, { 125, 900, 0.63 }, { 150, 1000, 1.0 } };
	data.m_big = model_core_set_create("core-BIG", perfs_big, 3);
	assert(data.m_big);

	model_core_set_perflvl_t perfs_little[] = { { 10, 200, 0.1 }, { 15, 300, 0.2 }, { 25, 500, 0.3 } };
	data.m_little = model_core_set_create("core-LITTLE", perfs_little, 3);
	assert(data.m_little);

	data.f = flowgraph_create("core selector", data.scoring, data.decision,
					  decision_callback, &data, 500000);
	assert(!flowgraph_attach_prediction(data.f, data.p_usage));
	assert(!flowgraph_attach_prediction(data.f, data.p_pwr));
	assert(!flowgraph_attach_model(data.f, data.m_big));
	assert(!flowgraph_attach_model(data.f, data.m_little));

	flowgraph_prediction_output_csv(data.f, "dvfs_%i_%s_%s.csv",
					flowgraph_prediction_output_csv_cb);
	flowgraph_model_output_csv(data.f, "dvfs_%i_model_%s_%s.csv",
			     flowgraph_model_csv_cb);
	scoring_output_csv(data.scoring, "dvfs_%s_%s.csv", scoring_output_csv_cb);

	do_work(argc, argv, data.me_usage, 10000000);

	flowgraph_teardown(data.f);

	fclose(data.log_decision);

	decision_delete(data.decision);
	model_delete(data.m_big);
	model_delete(data.m_little);
	scoring_delete(data.scoring);

	prediction_delete(data.p_pwr);
	prediction_delete(data.p_usage);

	metric_delete(data.me_usage);

	return 0;
}
