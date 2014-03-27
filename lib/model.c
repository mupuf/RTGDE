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

#include "model_priv.h"
#include <string.h>

static model_priv_t * model_priv(model_t *m)
{
	return (model_priv_t *)m;
}

model_t * model_create(model_exec_t exec, model_delete_t dtor, const char *name,
		       void *user)
{
	model_priv_t *m_priv = malloc(sizeof(model_priv_t));
	if (!m_priv)
		return NULL;

	m_priv->user = user;
	m_priv->exec = exec;
	m_priv->dtor = dtor;
	m_priv->name = strdup(name);

	return (model_t*) m_priv;
}

void *model_user(model_t *m)
{
	return model_priv(m)->user;
}

const char *model_name(model_t *m)
{
	return model_priv(m)->name;
}

decision_input_model_t *model_exec(model_t *m, prediction_list_t * predictions)
{
	decision_input_model_t *di = model_priv(m)->exec(m, predictions);
	prediction_list_delete(predictions);
	return di;
}

void model_delete(model_t *m)
{
	model_priv_t * m_priv = model_priv(m);

	if (m_priv->dtor)
		m_priv->dtor(m);
	if (m_priv->name)
		free(m_priv->name);

	free(m_priv);
}
