#include <stdio.h>
#include <assert.h>
#include <rtgde.h>
#include <stdlib.h>

#include <predictions/simple.h>
#include <models/dummy.h>

#define NETIF_DATARATE 6000000 // 48 MBit/s

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

	while (!pcap_read_packet_header(finput, &pkt))
	{
		uint64_t time_end = pkt.timestamp + pkt.len * 1000000 / NETIF_DATARATE;

		metric_update(me, pkt.timestamp - 1, 0);
		metric_update(me, pkt.timestamp, NETIF_DATARATE);
		metric_update(me, time_end, NETIF_DATARATE);
		metric_update(me, time_end + 1, 0);
	}

	/* wait for the prediction to be done and quit */
	sleep(1);
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

int main(int argc, char *argv[])
{
	prediction_t * mp = prediction_simple_create(1000);
	assert(mp);

	metric_t * me = metric_create("throughput", 1000);
	assert(me);

	assert(!prediction_attach_metric(mp, me));
	flowgraph_t *f = flowgraph_create("nif selector", 1000000);
	assert(!flowgraph_attach_prediction(f, mp));

	model_t * m = model_dummy_create();
	assert(m);

	assert(!flowgraph_attach_model(f, m));

	prediction_output_csv(mp, "pred_simple_%s_%i.csv");

	rtgde_start(f);

	do_work(argc, argv, me, 10000000);

	flowgraph_teardown(f);

	model_delete(m);

	prediction_delete(mp);

	metric_delete(me);

	return 0;
}