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
#include <signal.h>

#include <predictions/pred_fsm.h>
#include <predictions/constraint.h>
#include <scoring/simple.h>
#include "pcap_decision.h"
#include "pred_packets.h"
#include "model_simple_radio.h"
#include "utils.h"

#define DECISION_LOG_FILE "pcap_decision_log.csv"

volatile int request_quit = 0;

struct flowgraph_data {
	prediction_t * mp;
	metric_t * me_pkt;
	prediction_t * p_pwr;
	prediction_t * p_occ;
	prediction_t * p_lat;
	model_t * m_rwifi;
	model_t * m_rgsm;
	scoring_t * scoring;
	decision_t * decision;
	flowgraph_t *f;

	FILE *log_decision;
} data;

#ifdef HAS_LIBPCAP
#include <pcap.h>

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

int capture_packets(metric_t * me, int64_t timeout_us)
{
	pcap_t *handle;			/* Session handle */
	char *dev;			/* The device to sniff on */
	char errbuf[PCAP_ERRBUF_SIZE];	/* Error string */
	bpf_u_int32 mask;		/* Our netmask */
	bpf_u_int32 net;		/* Our IP */
	struct pcap_pkthdr header;	/* The header that pcap gives us */

	/* Define the device */
	dev = pcap_lookupdev(errbuf);
	if (dev == NULL) {
		fprintf(stderr, "Couldn't find default device: %s\n", errbuf);
		return(2);
	}
	/* Find the properties for the device */
	if (pcap_lookupnet(dev, &net, &mask, errbuf) == -1) {
		fprintf(stderr, "Couldn't get netmask for device %s: %s\n", dev, errbuf);
		net = 0;
		mask = 0;
	}
	/* Open the session in promiscuous mode */
	handle = pcap_open_live(dev, BUFSIZ, 1, 1000, errbuf);
	if (handle == NULL) {
		fprintf(stderr, "Couldn't open device %s: %s\n", "somedev", errbuf);
		return(2);
	}

	rtgde_start(data.f, 0);

	/* Grab a packet */
	metric_update(me, 0, 0);
	while (relative_time_us() < timeout_us && request_quit == 0) {
		pcap_next(handle, &header);

		metric_update(me, timeval_to_us(header.ts), header.len);
	}

	/* And close the session */
	pcap_close(handle);
	return(0);
}
#endif

struct pcap_packet {
	uint64_t timestamp;
	uint32_t len;
};

void die(const char *str)
{
	perror(str);
	exit(1);
}

int pcap_skip_header(FILE *file)
{
	uint32_t magic;

	fread(&magic, sizeof(uint32_t), 1, file);
	if (magic != 0xa1b2c3d4)
		return 1;

	fseek(file, 24, SEEK_SET);
	return 0;
}

int pcap_read_packet_header(FILE *file, struct pcap_packet *pkt)
{
	uint32_t ts_sec, ts_usec, incl_len, orig_len;
	int ret;

	ret = fread(&ts_sec, sizeof(uint32_t), 1, file);
	ret += fread(&ts_usec, sizeof(uint32_t), 1, file);
	ret += fread(&incl_len, sizeof(uint32_t), 1, file);
	ret += fread(&orig_len, sizeof(uint32_t), 1, file);
	if (ret != 4)
		return 1;

	fseek(file, incl_len, SEEK_CUR);

	pkt->timestamp = ts_sec * 1000000 + ts_usec;
	pkt->len = orig_len;

	return 0;
}

void read_from_file(metric_t * me, const char *filepath)
{
	struct pcap_packet pkt;

	FILE *finput = fopen(filepath, "rb");
	if (!finput)
		die("Cannot open the input file");

	if (pcap_skip_header(finput)) {
		fprintf(stderr, "Invalid magic number. The file '%s' isn't a pcap file\n", filepath);
	}

	rtgde_start(data.f, 0);

	int64_t last = 0, rel = 0;
	while (!pcap_read_packet_header(finput, &pkt) && request_quit == 0)
	{
		if (rel == 0)
			rel = pkt.timestamp - relative_time_us();

		if (pkt.timestamp <= last)
			pkt.timestamp = last + 1;
		last = pkt.timestamp;

		while (relative_time_us() < pkt.timestamp - rel) {
			usleep(100);
		}

		metric_update(me, pkt.timestamp, pkt.len);
	}
}

void usage(int argc, char **argv)
{
#ifdef HAS_LIBPCAP
	fprintf(stderr, "Usage: %s [filepath]\n", argv[0]);
#else
	fprintf(stderr, "Usage: %s filepath\n", argv[0]);
#endif
}

void do_work(int argc, char *argv[], metric_t * me, int64_t timeout_us)
{
#ifdef HAS_LIBPCAP
	if (argc == 1) {
		capture_packets(me, timeout_us);
		return;
	}
#endif

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
	const char *gnuplot_file = "../gnuplot/prediction.plot";
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
	const char *gnuplot_file = "../gnuplot/model_output_scored.plot";

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

	fprintf(stderr, "Gen %s\n", csv_filename);

	snprintf(cmd, sizeof(cmd), "gnuplot -e \"filename='%s'\" -e "
	       "\"graph_title='Score of the WiFi and GSM models for the %s metric.'\""
	       " ../gnuplot/score_log.plot", csv_filename, name);
	system(cmd);
}

void decision_callback(flowgraph_t *f, decision_input_t *di,
		       decision_input_model_t *dim, void *user)
{
	//struct flowgraph_data *d = (struct flowgraph_data*) user;
	decision_input_model_t *wifi, *gsm;

	if (!dim) {
		fprintf(stderr, "Callback decision: no decision has been made!\n");
		return;
	}

	wifi = decision_input_model_get_by_name(di, "radio-wifi");
	if (!wifi) {
		fprintf(stderr, "wifi model not found!\n");
		return;
	}

	gsm = decision_input_model_get_by_name(di, "radio-gsm");
	if (!gsm) {
		fprintf(stderr, "gsm model not found!\n");
		return;
	}

	fprintf(data.log_decision, "%" PRIu64 ", %f, %f, %f, %f, 0\n", relative_time_us(),
		wifi->score, gsm->score,
		wifi==dim?wifi->score:0, gsm==dim?gsm->score:0);
	fflush(data.log_decision);
	fsync(fileno(data.log_decision));

	system("gnuplot -e \"filename='"DECISION_LOG_FILE"'\" -e "
	       "\"graph_title='Decision result for the WiFi and GSM models'\""
	       " ../gnuplot/decision_log.plot");
}

int main(int argc, char *argv[])
{
	data.log_decision = fopen(DECISION_LOG_FILE, "w");
	if (!data.log_decision)  {
		perror("cannot open 'pcap_decision_log.csv'");
		return 0;
	}
	fprintf(data.log_decision, "time (µs), wifi model score, gsm model score, wifi selected, gsm selected, always 0\n");

	signal(SIGINT, sig_request_quit);
	signal(SIGQUIT, sig_request_quit);
	signal(SIGABRT, sig_request_quit);
	signal(SIGTERM, sig_request_quit);

	data.mp = pred_packets_create(1000000, 2);
	assert(data.mp);

	data.me_pkt = metric_create("packets", "bytes", 1000);
	assert(data.me_pkt);

	assert(!prediction_attach_metric(data.mp, data.me_pkt));

	data.p_pwr = prediction_constraint_create("power", "mW", 1000000, 0,
							  500, 3000, scoring_inverted);

	data.p_occ = prediction_constraint_create("RF-occupancy", "%", 1000000, 0,
							  50, 100, scoring_inverted);

	data.p_lat = prediction_constraint_create("nif-latency", "µs", 1000000, 0,
							  5, 10, scoring_inverted);

	data.scoring = score_simple_create();
	assert(data.scoring);

	assert(scoring_metric_create(data.scoring, "Power consumption", 20));
	assert(scoring_metric_create(data.scoring, "RF occupency", 3));
	assert(scoring_metric_create(data.scoring, "Emission latency", 5));

	data.decision = decision_pcap_create();
	assert(data.decision );

	data.m_rwifi = model_simple_radio_create("radio-wifi", 4800000, 0.001, 0.001,
						50, 10, 0.2, 0.3);
	assert(data.m_rwifi);

	data.m_rgsm = model_simple_radio_create("radio-gsm", 100000, 0.0001, 0.0001,
						70, 15, 0.01, 0.8);
	assert(data.m_rgsm);

	data.f = flowgraph_create("nif selector", data.scoring, data.decision,
					  decision_callback, &data, 1000000);
	assert(!flowgraph_attach_prediction(data.f, data.mp));
	assert(!flowgraph_attach_prediction(data.f, data.p_pwr));
	assert(!flowgraph_attach_prediction(data.f, data.p_occ));
	assert(!flowgraph_attach_prediction(data.f, data.p_lat));
	assert(!flowgraph_attach_model(data.f, data.m_rwifi));
	assert(!flowgraph_attach_model(data.f, data.m_rgsm));

	flowgraph_prediction_output_csv(data.f, "pcap_%i_%s_%s.csv",
					flowgraph_prediction_output_csv_cb);
	flowgraph_model_output_csv(data.f, "pcap_%i_model_%s_%s.csv",
			     flowgraph_model_csv_cb);
	scoring_output_csv(data.scoring, "pcap_score_%s_%s.csv", scoring_output_csv_cb);

	do_work(argc, argv, data.me_pkt, 10000000);

	flowgraph_teardown(data.f);

	fclose(data.log_decision);

	decision_delete(data.decision);
	model_delete(data.m_rgsm);
	model_delete(data.m_rwifi);
	scoring_delete(data.scoring);

	prediction_delete(data.p_lat);
	prediction_delete(data.p_occ);
	prediction_delete(data.p_pwr);
	prediction_delete(data.mp);

	metric_delete(data.me_pkt);

	return 0;
}
