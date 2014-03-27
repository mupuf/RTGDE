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

#ifndef MODEL_H
#define MODEL_H

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {

} model_t;

#include "prediction.h"
#include "decision_input.h"

typedef decision_input_model_t *(*model_exec_t)(model_t *m, prediction_list_t *prediction);
typedef void (*model_delete_t)(model_t *m);

model_t * model_create(model_exec_t exec, model_delete_t dtor, const char *name,
		       void *user);

void *model_user(model_t *m);
const char *model_name(model_t *m);
decision_input_model_t *model_exec(model_t *m, prediction_list_t * predictions);
void model_delete(model_t *m);

#ifdef __cplusplus
}
#endif

#endif // MODEL_H
