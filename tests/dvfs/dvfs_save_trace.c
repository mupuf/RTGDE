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
#include <unistd.h>
#include <inttypes.h>

#include <glibtop.h>
#include <glibtop/cpu.h>

int64_t clock_read_us()
{
	struct timespec tp;
	if (clock_gettime(CLOCK_REALTIME, &tp))
		return 0;

	return tp.tv_sec * 1000000ULL + tp.tv_nsec / 1000;
}

int64_t relative_time_us()
{
	static int64_t boot_time = 0;
	if (boot_time == 0)
		boot_time = clock_read_us();
	return clock_read_us() - boot_time;
}

FILE *file;

void sig_request_quit(int signal)
{
	fflush(file);
	fclose(file);
	exit(0);
}

int main(int argc, char *argv[])
{
	const char *filename;
	glibtop_cpu cpu, prev_cpu;

	signal(SIGINT, sig_request_quit);
	signal(SIGQUIT, sig_request_quit);
	signal(SIGABRT, sig_request_quit);
	signal(SIGTERM, sig_request_quit);

	if (argc == 2) {
		filename = argv[1];
		file = fopen(filename, "w");
		if (!file) {
			fprintf(stderr, "Cannot open '%s', bail out!\n", filename);
			return 1;
		}
	} else
		file = stdout;

	glibtop_init();

	glibtop_get_cpu (&prev_cpu);
	while (1) {
		usleep(100000);
		glibtop_get_cpu (&cpu);

		guint64 total_diff = cpu.total - prev_cpu.total;
		guint64 idle_diff = cpu.idle - prev_cpu.idle;
		int32_t usage = 100 - (idle_diff * 100 / total_diff);

		fprintf(file, "%" PRIu64 ", %i\n", relative_time_us(), usage);

		prev_cpu = cpu;
	}

	return 0;
}
