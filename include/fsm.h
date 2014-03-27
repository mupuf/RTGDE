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

#ifndef FSM_H
#define FSM_H

#ifdef __cplusplus
extern "C" {
#endif

#include "sample.h"

typedef struct {
} fsm_t;

typedef struct {
	char *name;
} fsm_state_t;

typedef fsm_state_t *(*fsm_next_state_t)(fsm_t *fsm, const char *metric, sample_value_t value);
typedef void (*fsm_dtor_t)(fsm_t *fsm);

fsm_t *fsm_create(fsm_next_state_t next_state, fsm_dtor_t dtor, void *user);
fsm_state_t *fsm_add_state(fsm_t *fsm, char *name, void *user);
fsm_state_t *fsm_update_state(fsm_t *fsm, const char *metric, sample_value_t value);
void *fsm_get_user(fsm_t *fsm);
void *fsm_state_get_user(fsm_state_t *fsm_state);
void fsm_delete(fsm_t *fsm);

#ifdef __cplusplus
}
#endif

#endif // FSM_H
