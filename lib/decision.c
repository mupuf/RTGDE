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

#include "decision_priv.h"
#include <string.h>

static decision_priv_t *decision_priv(decision_t *s)
{
	return (decision_priv_t *)s;
}

decision_t *decision_create(decision_exec_t exec, decision_dtor_t dtor,
			    const char *name, void *user)
{
	decision_priv_t *d_priv = malloc(sizeof(decision_priv_t));
	if (!d_priv)
		return NULL;

	d_priv->exec = exec;
	d_priv->dtor = dtor;
	d_priv->name = strdup(name);
	d_priv->user = user;

	return (decision_t *)d_priv;
}

const char *decision_name(decision_t *d)
{
	return decision_priv(d)->name;
}

void *decision_user(decision_t *d)
{
	return decision_priv(d)->user;
}

decision_input_model_t *decision_exec(decision_t *d, decision_input_t *di)
{
	decision_priv_t *d_priv = decision_priv(d);
	if (d_priv->exec)
		return d_priv->exec(d, di);
	return NULL;
}

void decision_delete(decision_t *d)
{
	decision_priv_t *d_priv = decision_priv(d);
	if (d_priv->dtor)
		d_priv->dtor(d);
	free((char *)d_priv->name);
	free(d);
}
