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

#ifndef DECISION_H
#define DECISION_H

#include "list.h"
#include "model.h"
#include "prediction.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
	const char *name;
	void *user;
} decision_t;

typedef decision_input_model_t *(*decision_exec_t)(decision_t *d, decision_input_t *di);
typedef void (*decision_dtor_t)(decision_t *d);

decision_t *decision_create(decision_exec_t exec, decision_dtor_t dtor,
			    const char *name, void *user);
void *decision_user(decision_t *d);
void decision_call_user_cb(decision_t *d, model_t *m);
const char *decision_name(decision_t *d);
decision_input_model_t *decision_exec(decision_t *d, decision_input_t *di);
void decision_delete(decision_t *d);

void decision_output_csv(decision_t *d, const char *csv_filename);


#ifdef __cplusplus
}
#endif

#endif // DECISION_H
