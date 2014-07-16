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
#include <rtgde.h>
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <predictions/simple.h>
#include <predictions/average.h>
#include <models/dummy.h>
#include <scoring/simple.h>
#include <decision/simple.h>
#include "utils.h"

int64_t relative_time_us()
{
	static int64_t boot_time = 0;
	if (boot_time == 0)
		boot_time = clock_read_us();
	return clock_read_us() - boot_time;
}

struct flowgraph_data {
	flowgraph_t *f;
} data;

void decision_callback(flowgraph_t *f, decision_input_t *di,
		       decision_input_model_t *dim, void *user)
{

}

int main(int argc, char **argv)
{
	int i;

	data.f = flowgraph_create("minimal", NULL, NULL,
				  decision_callback, &data, 1000000);

	rtgde_start(data.f, 0);

	sleep(30);

	flowgraph_teardown(data.f);

	return 0;
}
