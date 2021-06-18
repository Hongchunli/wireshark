/*
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 2001 Gerald Combs
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "config.h"

#include "syntax-tree.h"
#include <wsutil/ws_assert.h>

/* Keep track of sttype_t's via their sttype_id_t number */
static sttype_t* type_list[STTYPE_NUM_TYPES];


#define STNODE_MAGIC	0xe9b00b9e


void
sttype_init(void)
{
	sttype_register_function();
	sttype_register_integer();
	sttype_register_pointer();
	sttype_register_range();
	sttype_register_set();
	sttype_register_string();
	sttype_register_test();
}

void
sttype_cleanup(void)
{
	/* nothing to do */
}


void
sttype_register(sttype_t *type)
{
	sttype_id_t	type_id;

	type_id = type->id;

	/* Check input */
	ws_assert(type_id < STTYPE_NUM_TYPES);

	/* Don't re-register. */
	ws_assert(type_list[type_id] == NULL);

	type_list[type_id] = type;
}

static sttype_t*
sttype_lookup(sttype_id_t type_id)
{
	sttype_t	*result;

	/* Check input */
	ws_assert(type_id < STTYPE_NUM_TYPES);

	result = type_list[type_id];

	/* Check output. */
	ws_assert(result != NULL);

	return result;
}


stnode_t*
stnode_new(sttype_id_t type_id, gpointer data)
{
	sttype_t	*type;
	stnode_t	*node;

	node = g_new(stnode_t, 1);
	node->magic = STNODE_MAGIC;
	node->deprecated_token = NULL;
	node->inside_brackets = FALSE;

	if (type_id == STTYPE_UNINITIALIZED) {
		node->type = NULL;
		node->data = NULL;
	}
	else {
		type = sttype_lookup(type_id);
		ws_assert(type);
		node->type = type;
		if (type->func_new) {
			node->data = type->func_new(data);
		}
		else {
			node->data = data;
		}

	}

	return node;
}

void
stnode_set_bracket(stnode_t *node, gboolean bracket)
{
	node->inside_brackets = bracket;
}

stnode_t*
stnode_dup(const stnode_t *org)
{
	sttype_t	*type;
	stnode_t	*node;

	if (!org)
		return NULL;

	type = org->type;

	node = g_new(stnode_t, 1);
	node->magic = STNODE_MAGIC;
	node->deprecated_token = NULL;
	node->type = type;
	if (type && type->func_dup)
		node->data = type->func_dup(org->data);
	else
		node->data = org->data;
	node->value = org->value;
	node->inside_brackets = org->inside_brackets;

	return node;
}

void
stnode_init(stnode_t *node, sttype_id_t type_id, gpointer data)
{
	sttype_t	*type;

	ws_assert_magic(node, STNODE_MAGIC);
	ws_assert(!node->type);
	ws_assert(!node->data);

	type = sttype_lookup(type_id);
	ws_assert(type);
	node->type = type;
	if (type->func_new) {
		node->data = type->func_new(data);
	}
	else {
		node->data = data;
	}
}

void
stnode_init_int(stnode_t *node, sttype_id_t type_id, gint32 value)
{
	stnode_init(node, type_id, NULL);
	node->value = value;
}

void
stnode_free(stnode_t *node)
{
	ws_assert_magic(node, STNODE_MAGIC);
	if (node->type) {
		if (node->type->func_free) {
			node->type->func_free(node->data);
		}
	}
	else {
		ws_assert(!node->data);
	}
	g_free(node);
}

const char*
stnode_type_name(stnode_t *node)
{
	ws_assert_magic(node, STNODE_MAGIC);
	if (node->type)
		return node->type->name;
	else
		return "UNINITIALIZED";
}

sttype_id_t
stnode_type_id(stnode_t *node)
{
	ws_assert_magic(node, STNODE_MAGIC);
	if (node->type)
		return node->type->id;
	else
		return STTYPE_UNINITIALIZED;
}

gpointer
stnode_data(stnode_t *node)
{
	ws_assert_magic(node, STNODE_MAGIC);
	return node->data;
}

gpointer
stnode_steal_data(stnode_t *node)
{
	ws_assert_magic(node, STNODE_MAGIC);
	gpointer data = node->data;
	ws_assert(data);
	node->data = NULL;
	return data;
}

gint32
stnode_value(stnode_t *node)
{
	ws_assert_magic(node, STNODE_MAGIC);
	return node->value;
}

const char *
stnode_deprecated(stnode_t *node)
{
	if (!node) {
		return NULL;
	}
	return node->deprecated_token;
}

/*
 * Editor modelines  -  https://www.wireshark.org/tools/modelines.html
 *
 * Local variables:
 * c-basic-offset: 8
 * tab-width: 8
 * indent-tabs-mode: t
 * End:
 *
 * vi: set shiftwidth=8 tabstop=8 noexpandtab:
 * :indentSize=8:tabSize=8:noTabs=false:
 */
