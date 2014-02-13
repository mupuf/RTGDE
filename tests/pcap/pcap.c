#include <stdio.h>
#include <assert.h>
#include <rtgde.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <predictions/pred_fsm.h>
#include <predictions/constraint.h>
#include "pred_packets.h"
#include "model_simple_radio.h"

#define NETIF_DATARATE 12500000 // 100 MBit/s

#ifdef HAS_LIBPCAP
#include <pcap.h>

int64_t relative_time_us()
{
	static int64_t boot_time = 0;
	if (boot_time == 0)
		boot_time = clock_read_us();
	return clock_read_us() - boot_time;
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

	/* Grab a packet */
	metric_update(me, 0, 0);
	while (relative_time_us() < timeout_us) {
		pcap_next(handle, &header);
		int64_t time = relative_time_us();

		metric_update(me, time - 1, 0);
		metric_update(me, time, NETIF_DATARATE);
		metric_update(me, time + header.len * 1000000 / NETIF_DATARATE, NETIF_DATARATE);
		metric_update(me, time + (header.len * 1000000 / NETIF_DATARATE) + 1, 0);
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

	sample_time_t last = 0;
	while (!pcap_read_packet_header(finput, &pkt))
	{
		if (pkt.len < 200)
			continue;

		if (pkt.timestamp <= last)
			pkt.timestamp = last + 1;

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
	if (value > 0)
		return fsm_pred_data.on.fsm_st;
	else
		return fsm_pred_data.off.fsm_st;
}

int fsm_pred_throuput_metric_from_state(fsm_state_t *state,
					const char *metric_name,
					sample_value_t *value)
{
	struct fsm_pred_throuput_state *st = fsm_state_get_user(state);
	if (strcmp(metric_name, "throughput") == 0) {
		*value = st->power;
		return 0;
	} else
		return 1;
}

void flowgraph_output_csv_cb(flowgraph_t *f, decision_input_metric_t* m,
					  const char *csv_filename)
{
	char cmd[1024];
	const char *gnuplot_file = "../gnuplot/metric_overview.plot";

	if (m->prediction->scoring_style == scoring_inverted) {
		gnuplot_file = "../gnuplot/metric_overview_inverted.plot";
	}

	snprintf(cmd, sizeof(cmd),
		 "gnuplot -e \"filename='%s'\" -e \"graph_title='%s'\" %s",
		 csv_filename, m->name, gnuplot_file);
	system(cmd);

}

int main(int argc, char *argv[])
{
	fsm_t *pred_fsm = fsm_create(fsm_pred_throuput_next_state, NULL, &fsm_pred_data);
	fsm_pred_data.on.fsm_st = fsm_add_state(pred_fsm, "ON", &fsm_pred_data.on);
	fsm_pred_data.off.fsm_st = fsm_add_state(pred_fsm, "OFF", &fsm_pred_data.off);
	fsm_pred_data.cur = fsm_pred_data.off.fsm_st;

	/*prediction_t * mp = prediction_fsm_create(pred_fsm,
						  fsm_pred_throuput_metric_from_state,
						  250000, 10);
	assert(mp);*/
	prediction_t * mp = pred_packets_create(1000000, 2);
	assert(mp);

	metric_t * me = metric_create("packets", 1000);
	assert(me);

	assert(!prediction_attach_metric(mp, me));

	prediction_t * mpc = prediction_constraint_create("power", 1000000, 0,
							  500, 1000, scoring_inverted);

	prediction_t * mpo = prediction_constraint_create("RF-occupancy", 1000000, 0,
							  50, 100, scoring_inverted);

	prediction_t * mpl = prediction_constraint_create("nif-latency", 1000000, 0,
							  5, 10, scoring_inverted);

	scoring_t * scoring = score_simple_create();
	assert(scoring);

	assert(scoring_metric_create(scoring, "Power consumption", 10));
	assert(scoring_metric_create(scoring, "RF occupency", 3));
	assert(scoring_metric_create(scoring, "Emission latency", 100));

	flowgraph_t *f = flowgraph_create("nif selector", scoring, NULL,
					  NULL, NULL, 1000000);
	assert(!flowgraph_attach_prediction(f, mp));
	assert(!flowgraph_attach_prediction(f, mpc));
	assert(!flowgraph_attach_prediction(f, mpo));
	assert(!flowgraph_attach_prediction(f, mpl));

	model_t * m = model_simple_radio_create("radio1", 10000, 0.001, 0.001,
						100, 20, 0.1, 0.2);
	assert(m);

	assert(!flowgraph_attach_model(f, m));

	flowgraph_output_csv(f, "pcap_%s_%s_%i.csv", flowgraph_output_csv_cb);

	do_work(argc, argv, me, 10000000);

	rtgde_start(f, 1);

	flowgraph_teardown(f);

	model_delete(m);

	scoring_delete(scoring);

	prediction_delete(mpl);
	prediction_delete(mpo);
	prediction_delete(mpc);
	prediction_delete(mp);

	metric_delete(me);

	return 0;
}
