/*
 * Copyright (c) 2015 Open Grid Computing, Inc. All rights reserved.
 * Copyright (c) 2015 Sandia Corporation. All rights reserved.
 *
 * Under the terms of Contract DE-AC04-94AL85000, there is a non-exclusive
 * license for use of this work by or on behalf of the U.S. Government.
 * Export of this program may require a license from the United States
 * Government.
 *
 * This software is available to you under a choice of one of two
 * licenses.  You may choose to be licensed under the terms of the GNU
 * General Public License (GPL) Version 2, available from the file
 * COPYING in the main directory of this source tree, or the BSD-type
 * license below:
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *      Redistributions of source code must retain the above copyright
 *      notice, this list of conditions and the following disclaimer.
 *
 *      Redistributions in binary form must reproduce the above
 *      copyright notice, this list of conditions and the following
 *      disclaimer in the documentation and/or other materials provided
 *      with the distribution.
 *
 *      Neither the name of Sandia nor the names of any contributors may
 *      be used to endorse or promote products derived from this software
 *      without specific prior written permission.
 *
 *      Neither the name of Open Grid Computing nor the names of any
 *      contributors may be used to endorse or promote products derived
 *      from this software without specific prior written permission.
 *
 *      Modified source versions must be plainly marked as such, and
 *      must not be misrepresented as being the original software.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 * A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 * OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 * LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Author: Tom Tucker tom at ogc dot us
 */

#include <sys/types.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include <assert.h>
#include <string.h>
#include <sos/sos.h>
#include "sos_priv.h"

static uint32_t type_sizes[] = {
	[SOS_TYPE_INT32] = 4,
	[SOS_TYPE_INT64] = 8,
	[SOS_TYPE_UINT32] = 4,
	[SOS_TYPE_UINT64] = 8,
	[SOS_TYPE_FLOAT] = 4,
	[SOS_TYPE_DOUBLE] = 8,
	[SOS_TYPE_LONG_DOUBLE] = 16,
	[SOS_TYPE_TIMESTAMP] = 8,
	[SOS_TYPE_OBJ] = 16,
	[SOS_TYPE_BYTE_ARRAY] = 8,
	[SOS_TYPE_INT32_ARRAY] = 8,
	[SOS_TYPE_INT64_ARRAY] = 8,
	[SOS_TYPE_UINT32_ARRAY] = 8,
	[SOS_TYPE_UINT64_ARRAY] = 8,
	[SOS_TYPE_FLOAT_ARRAY] = 8,
	[SOS_TYPE_DOUBLE_ARRAY] = 8,
	[SOS_TYPE_LONG_DOUBLE_ARRAY] = 8,
	[SOS_TYPE_OBJ_ARRAY] = 8,
};

static uint32_t element_sizes[] = {
	[SOS_TYPE_INT32] = 4,
	[SOS_TYPE_INT64] = 8,
	[SOS_TYPE_UINT32] = 4,
	[SOS_TYPE_UINT64] = 8,
	[SOS_TYPE_FLOAT] = 4,
	[SOS_TYPE_DOUBLE] = 8,
	[SOS_TYPE_LONG_DOUBLE] = 16,
	[SOS_TYPE_OBJ] = 16,
	[SOS_TYPE_BYTE_ARRAY] = 1,
	[SOS_TYPE_INT32_ARRAY] = 4,
	[SOS_TYPE_INT64_ARRAY] = 8,
	[SOS_TYPE_UINT32_ARRAY] = 4,
	[SOS_TYPE_UINT64_ARRAY] = 8,
	[SOS_TYPE_FLOAT_ARRAY] = 4,
	[SOS_TYPE_DOUBLE_ARRAY] = 8,
	[SOS_TYPE_LONG_DOUBLE_ARRAY] = 16,
	[SOS_TYPE_OBJ_ARRAY] = 8,
};

/**
 * \page schema_overview Schema Overview
 *
 * Objects in a SOS container are described by a schema. Every object
 * in a SOS database is associated with a schema. A container may have
 * any number of schema and therefore objects of different types can
 * be stored in the same container. A schema consists of a unique name
 * and a set of attribute specifications that describe the object.
 *
 * Schema are created with the sos_schema_new() function. After a
 * schema is created it must be associated with one or more containers
 * with the sos_schema_add() function. Once a schema has been added to
 * a container, it can be used to create objects using the
 * sos_obj_new() function.
 *
 * Attributes are identified by their name and by the order in which
 * they were added to the schema. To manipulate or query a schema
 * attribute the attribute handle is required. The attribute handle is
 * returned by the sos_schema_attr_by_id() or sos_schema_attr_by_name()
 * functions.
 *
 * - sos_schema_new()	     Create a schema
 * - sos_schema_attr_add()   Add an attribute to a schema
 * - sos_schema_index_add()  Add an index to an attribute
 * - sos_schema_add()	     Add a schema to a container
 * - sos_schema_find()	     Find the schema with the specified name
 * - sos_schema_delete()     Remove a schema from the container
 * - sos_schema_count()	     Returns the number of schema in the container
 * - sos_schema_get()        Take a reference on a schema
 * - sos_schema_put()        Drop a reference on a schema
 * - sos_schema_name()	     Get the schema's name
 * - sos_schema_attr_count() Returns the number of attributes in the schema.
 * - sos_schema_attr_by_id() Returns the attribute by ordinal id
 * - sos_schema_attr_by_name() Returns the attribute with the specified name
 */

/** \defgroup schema_funcs Schema Functions
 * @{
 */
/**
 * \brief Create a schema
 *
 * A schema defines a SOS object. Every object in a SOS database is
 * associated with a schema via an internal schema_id.
 *
 * After a schema is created it must be associated with one or more
 * containers. See the sos_schema_add() function to add a schema to a
 * container so that objects of that type can subsequently be created
 * in the container. Once a schema has been added, it can be looked up
 * with the sos_schema_by_name() and sos_schema_by_id() functions.
 *
 * Objects are created with the sos_obj_new() function. This function
 * takes a schema-handle as it's argument. The schema-id is saved
 * internally with the object data. An object is therefore
 * self-describing.
 *
 * \param name	The name of the schema. This name must be unique
 * within the container.
 * \returns	A pointer to the new schema or a NULL pointer if there
 * is an error. The errno variable is set to provide detail on the error.
 */
sos_schema_t sos_schema_new(const char *name)
{
	sos_schema_t schema;

	if (strlen(name) >= SOS_SCHEMA_NAME_LEN)
		return NULL;
	schema = calloc(1, sizeof(*schema));
	if (schema) {
		schema->data = &schema->data_;
		strcpy(schema->data->name, name);
		TAILQ_INIT(&schema->attr_list);
	}
	schema->ref_count = 1;
	return schema;
}

void __sos_schema_free(sos_schema_t schema)
{
	/* Drop our reference on the schema object */
	if (schema->schema_obj)
		ods_obj_put(schema->schema_obj);

	/* Free all of our attributes and close it's indices */
	while (!TAILQ_EMPTY(&schema->attr_list)) {
		sos_attr_t attr = TAILQ_FIRST(&schema->attr_list);
		TAILQ_REMOVE(&schema->attr_list, attr, entry);
		sos_idx_part_t part;
		TAILQ_FOREACH(part, &attr->idx_list, entry)
			ods_idx_close(part->index, ODS_COMMIT_ASYNC);
		free(attr->key_type);
		free(attr->idx_type);
		free(attr);
	}
	free(schema);
}

/**
 * \brief Create a schema from a SOS template
 *
 * This convenience function provides a service that creates a SOS
 * schema from a structure that specifies the schema name and an array
 * of attribute definitions. For example:
 *
 *   struct sos_schema_template employee = {
 *       .name = "employee",
 *       .attrs = {
 *            { "First", SOS_TYPE_BYTE_ARRAY },
 *            { "LAST", SOS_TYPE_BYTE_ARRAY, 1 },
 *            { "Salary", SOS_TYPE_FLOAT },
 *            { NULL } // terminates attribute list
 *       }
 *   };
 *   sos_schema_t schema = sos_schema_from_template(&employee);
 *
 * \param t   Pointer to a template structure
 * \retval sos_schema_t created from the structure definition.
 */
sos_schema_t sos_schema_from_template(sos_schema_template_t t)
{
	int i, rc;
	sos_schema_t schema = sos_schema_new(t->name);
	if (!schema)
		goto err;
	for (i = 0; 1; i++) {
		if (!t->attrs[i].name)
			break;
		rc = sos_schema_attr_add(schema,
					 t->attrs[i].name, t->attrs[i].type);
		if (rc)
			goto err;
		if (t->attrs[i].indexed) {
			rc = sos_schema_index_add(schema, t->attrs[i].name);
			if (rc)
				goto err;
		}
	}
	return schema;
 err:
	__sos_schema_free(schema);
	return NULL;
}

/**
 * \brief Return the number of attributes in the schema.
 *
 * This function returns the number of attributes in the schema. See
 * the sos_schema_attr_by_id() function for an example that iterates
 * through all attributes defined in the schema.
 *
 * \param schema The schema handle.
 * \retval The number of attributes in the schema.
 */
int sos_schema_attr_count(sos_schema_t schema)
{
	return schema->data->attr_cnt;
}

/**
 * \brief Returns the schema's name
 * \param schema The schema handle.
 * \returns The schema's name.
 */
const char *sos_schema_name(sos_schema_t schema)
{
	return schema->data->name;
}

int sos_schema_id(sos_schema_t schema)
{
	return schema->data->id;
}

/**
 * \brief Return number of schema in container
 *
 * \param sos The container handle
 * \retval The number of schema in the container
 */
size_t sos_schema_count(sos_t sos)
{
	return sos->schema_count;
}

/**
 * \brief Take a reference on a schema
 *
 * SOS schema are reference counted. This function takes a reference
 * on a SOS schema and returns a pointer to the same. The typical
 * calling sequence is:
 *
 *     sos_schema_t my_schema_ptr = sos_schema_get(schema);
 *
 *
 * This allows for the schema to be safely pointed to from multiple
 * places. The sos_schema_put() function is used to drop a reference on
 * the schema. For example:
 *
 *     sos_schema_put(my_schema_ptr);
 *     my_schema_ptr = NULL;
 *
 * \param schema	The SOS schema handle
 * \returns The schema handle
 */
sos_schema_t sos_schema_get(sos_schema_t schema)
{
	ods_atomic_inc(&schema->ref_count);
	return schema;
}

/**
 * \brief Drop a reference on a schema
 *
 * The memory consumed by a schema is not released until all
 * references have been dropped.
 *
 * \param schema	The schema handle
 */
void sos_schema_put(sos_schema_t schema)
{
	/* Ignore for internal schemas */
	if (!schema || (schema->flags & SOS_SCHEMA_F_INTERNAL))
		return;
	if (!ods_atomic_dec(&schema->ref_count))
		__sos_schema_free(schema);
}

const char *key_types[] = {
	[SOS_TYPE_INT32] = "INT32",
	[SOS_TYPE_INT64] = "INT64",
	[SOS_TYPE_UINT32] = "UINT32",
	[SOS_TYPE_UINT64] = "UINT64",
	[SOS_TYPE_FLOAT] = "FLOAT",
	[SOS_TYPE_DOUBLE] = "DOUBLE",
	[SOS_TYPE_LONG_DOUBLE] = "LONG_DOUBLE",
	[SOS_TYPE_TIMESTAMP] = "UINT64",
	[SOS_TYPE_OBJ] = "NONE",
	[SOS_TYPE_BYTE_ARRAY] = "STRING",
 	[SOS_TYPE_INT32_ARRAY] = "NONE",
	[SOS_TYPE_INT64_ARRAY] = "NONE",
	[SOS_TYPE_UINT32_ARRAY] = "NONE",
	[SOS_TYPE_UINT64_ARRAY] = "NONE",
	[SOS_TYPE_FLOAT_ARRAY] = "NONE",
	[SOS_TYPE_DOUBLE_ARRAY] = "NONE",
	[SOS_TYPE_LONG_DOUBLE_ARRAY] = "NONE",
	[SOS_TYPE_OBJ_ARRAY] = "NONE",
};

static sos_attr_t attr_new(sos_schema_t schema, sos_type_t type)
{
	sos_attr_t attr = calloc(1, sizeof *attr);
	if (!attr)
		goto out;
	attr->data = &attr->data_;
	attr->schema = schema;
	TAILQ_INIT(&attr->idx_list);
	attr->size_fn = __sos_attr_size_fn_for_type(type);
	attr->to_str_fn = __sos_attr_to_str_fn_for_type(type);
	attr->from_str_fn = __sos_attr_from_str_fn_for_type(type);
	attr->key_value_fn = __sos_attr_key_value_fn_for_type(type);
	attr->idx_type = strdup("BXTREE");
	attr->key_type = strdup(key_types[type]);
 out:
	return attr;
}

static sos_attr_t _attr_by_name(sos_schema_t schema, const char *name)
{
	sos_attr_t attr;
	TAILQ_FOREACH(attr, &schema->attr_list, entry) {
		if (0 == strcmp(name, attr->data->name))
			return attr;
	}
	return NULL;
}

static sos_attr_t _attr_by_idx(sos_schema_t schema, int attr_id)
{
	sos_attr_t attr;
	if (attr_id < 0 || attr_id >= schema->data->attr_cnt)
		return NULL;
	if (schema->dict)
		return schema->dict[attr_id];
	TAILQ_FOREACH(attr, &schema->attr_list, entry) {
		if (attr->data->id == attr_id)
			return attr;
	}
	return NULL;
}

/**
 * \brief Add an attribute to a schema
 *
 * Add an attribute to a schema. The schema can only be modified if it
 * is not already a member of the container.
 *
 * \param schema	The schema
 * \param name		The attribute name
 * \param type		The attribute type
 * \retval 0		Success
 * \retval ENOMEM	Insufficient resources
 * \retval EEXIST	An attribute with that name already exists
 * \retval EINUSE	The schema is already a member of a container
 * \retval EINVAL	A parameter was invalid
 */
int sos_schema_attr_add(sos_schema_t schema, const char *name, sos_type_t type)
{
	sos_attr_t attr;
	sos_attr_t prev = NULL;

	if (schema->schema_obj)
		return EBUSY;

	/* Search the schema to see if the name is already in use */
	attr = _attr_by_name(schema, name);
	if (attr)
		return EEXIST;

	if (type > SOS_TYPE_LAST)
		return EINVAL;

	if (!TAILQ_EMPTY(&schema->attr_list))
		prev = TAILQ_LAST(&schema->attr_list, sos_attr_list);

	attr = attr_new(schema, type);
	if (!attr)
		return ENOMEM;
	strcpy(attr->data->name, name);
	attr->data->type = type;
	attr->data->id = schema->data->attr_cnt++;
	if (prev)
		attr->data->offset = prev->data->offset + type_sizes[prev->data->type];
	else
		attr->data->offset = sizeof(struct sos_obj_data_s);

	uint32_t a_size = sos_attr_size(attr);
	if (a_size > schema->data->key_sz)
		schema->data->key_sz = a_size;
	schema->data->obj_sz = attr->data->offset + type_sizes[attr->data->type];

	/* Append new attribute to tail of list */
	TAILQ_INSERT_TAIL(&schema->attr_list, attr, entry);
	return 0;
}

/**
 * \brief Add an index to an attribute
 *
 * Marks an attribute as having a key-value index. The index is not
 * actually created until the schema containing the attribute is added
 * to the container.
 *
 * \param schema	The schema handle
 * \param name		The attribute name
 * \retval 0		The index was succesfully added.
 * \retval ENOENT	The specified attribute does not exist.
 * \retval EINVAL	One or more parameters was invalid.
 */
int sos_schema_index_add(sos_schema_t schema, const char *name)
{
	sos_attr_t attr;

	if (schema->schema_obj)
		return EBUSY;

	/* Find the attribute */
	attr = _attr_by_name(schema, name);
	if (!attr)
		return ENOENT;

	attr->data->indexed = 1;
	return 0;
}

/**
 * \brief Modify the index for an attribute
 *
 * By default an attribute index is a modified form of a B+Tree that
 * efficiently handles duplicate keys. There are, however, other index
 * types as well as user-defined indices. The type of index is
 * specified as a string that identifies a shared library that
 * implements the necessary index strategy routines, e.g. insert,
 * delete, etc...
 *
 * For keys, the default key type is associated with the attribute's
 * data type, however, it is possible to implement user-defined
 * key types. These are useful for indexing complex data types that
 * are not understood as primitive types; for example a set of fields
 * that are represented as a UINT64.

 * \param schema	The schema handle.
 * \param name		The attribute name.
 * \param idx_type	The index type name. This parameter cannot be null.
 * \param key_type	The key type name. It may be null, in which
 *			case, the default key type for the attribute will be used.
 * \param ...		Some index types have additional parameters,
 *			for example, the BXTREE has an order parameter
 *			that specifies the number of entries in each BXTREE node.
 * \retval 0		The index was succesfully added.
 * \retval ENOENT	The specified attribute does not exist.
 * \retval EINVAL	One or more parameters was invalid.
 */
int sos_schema_index_modify(sos_schema_t schema, const char *name,
			    const char *idx_type, const char *key_type, ...)
{
	sos_attr_t attr;

	if (schema->schema_obj)
		return EBUSY;

	/* Find the attribute */
	attr = _attr_by_name(schema, name);
	if (!attr)
		return ENOENT;

	attr->idx_type = strdup(idx_type);
	attr->key_type = strdup(key_type);
	return 0;
}

/**
 * \brief Find an attribute by name
 * \param schema	The schema handle
 * \param name		The attribute's name
 * \returns The attribute handle or NULL if the attribute was not found.
 */
sos_attr_t sos_schema_attr_by_name(sos_schema_t schema, const char *name)
{
	return _attr_by_name(schema, name);
}

/**
 * \brief Find an attribute by id
 *
 * This function is useful for iterating through all attributes in the
 * schema as shown in the following code fragment:
 *
 *     for (i = 0; i < sos_schema_attr_count(schema); i++) {
 *        sos_attr_t attr = sos_schema_attr_by_id(i);
 *        ... code to manipulate attribute ...
 *     }
 *
 * \param schema	The schema handle
 * \param attr_id	The attribute's ordinal id
 * \returns The attribute handle or NULL if the attribute was not found.
 */
sos_attr_t sos_schema_attr_by_id(sos_schema_t schema, int attr_id)
{
	return _attr_by_idx(schema, attr_id);
}

sos_attr_t sos_schema_attr_first(sos_schema_t schema)
{
	return TAILQ_FIRST(&schema->attr_list);
}

sos_attr_t sos_schema_attr_last(sos_schema_t schema)
{
	return TAILQ_LAST(&schema->attr_list, sos_attr_list);
}

sos_attr_t sos_schema_attr_next(sos_attr_t attr)
{
	return TAILQ_NEXT(attr, entry);
}

sos_attr_t sos_schema_attr_prev(sos_attr_t attr)
{
	return TAILQ_PREV(attr, sos_attr_list, entry);
}

sos_type_t sos_attr_type(sos_attr_t attr)
{
	return attr->data->type;
}

int sos_attr_id(sos_attr_t attr)
{
	return attr->data->id;
}

const char *sos_attr_name(sos_attr_t attr)
{
	return attr->data->name;
}

sos_schema_t sos_attr_schema(sos_attr_t attr)
{
	return attr->schema;
}

struct sos_schema_s sos_obj_ischema = {
	.sos = NULL,
	.flags = SOS_SCHEMA_F_INTERNAL,
	.ref_count = 1,
	.data = &sos_obj_ischema.data_,
	.data_ = {
		.name = "ISCHEMA_OBJ",
		.ref_count = 1,
		.id = SOS_ISCHEMA_OBJ,
		.obj_sz = sizeof(ods_ref_t),
	},
};
struct sos_schema_s sos_byte_array_ischema = {
	.sos = NULL,
	.flags = SOS_SCHEMA_F_INTERNAL,
	.ref_count = 1,
	.data = &sos_byte_array_ischema.data_,
	.data_ = {
		.name = "BYTE_ARRAY",
		.ref_count = 1,
		.id = SOS_ISCHEMA_BYTE_ARRAY,
		.obj_sz = sizeof(char),
	},
};
struct sos_schema_s sos_int32_array_ischema = {
	.sos = NULL,
	.flags = SOS_SCHEMA_F_INTERNAL,
	.ref_count = 1,
	.data = &sos_int32_array_ischema.data_,
	.data_ = {
		.name = "INT32_ARRAY",
		.ref_count = 1,
		.id = SOS_ISCHEMA_INT32_ARRAY,
		.obj_sz = sizeof(int32_t),
	},
};
struct sos_schema_s sos_int64_array_ischema = {
	.sos = NULL,
	.flags = SOS_SCHEMA_F_INTERNAL,
	.ref_count = 1,
	.data = &sos_int64_array_ischema.data_,
	.data_ = {
		.name = "INT64_ARRAY",
		.ref_count = 1,
		.id = SOS_ISCHEMA_INT64_ARRAY,
		.obj_sz = sizeof(int64_t),
	},
};
struct sos_schema_s sos_uint32_array_ischema = {
	.sos = NULL,
	.flags = SOS_SCHEMA_F_INTERNAL,
	.ref_count = 1,
	.data = &sos_uint32_array_ischema.data_,
	.data_ = {
		.name = "UINT32_ARRAY",
		.ref_count = 1,
		.id = SOS_ISCHEMA_BYTE_ARRAY,
		.obj_sz = sizeof(uint32_t),
	},
};
struct sos_schema_s sos_uint64_array_ischema = {
	.sos = NULL,
	.flags = SOS_SCHEMA_F_INTERNAL,
	.ref_count = 1,
	.data = &sos_uint64_array_ischema.data_,
	.data_ = {
		.name = "UINT64_ARRAY",
		.ref_count = 1,
		.id = SOS_ISCHEMA_UINT64_ARRAY,
		.obj_sz = sizeof(uint64_t),
	},
};
struct sos_schema_s sos_float_array_ischema = {
	.sos = NULL,
	.flags = SOS_SCHEMA_F_INTERNAL,
	.ref_count = 1,
	.data = &sos_float_array_ischema.data_,
	.data_ = {
		.name = "ISCHEMA_FLOAT_ARRAY",
		.ref_count = 1,
		.id = SOS_ISCHEMA_FLOAT_ARRAY,
		.obj_sz = sizeof(float),
	},
};
struct sos_schema_s sos_double_array_ischema = {
	.sos = NULL,
	.flags = SOS_SCHEMA_F_INTERNAL,
	.ref_count = 1,
	.data = &sos_double_array_ischema.data_,
	.data_ = {
		.name = "DOUBLE_ARRAY",
		.ref_count = 1,
		.id = SOS_ISCHEMA_DOUBLE_ARRAY,
		.obj_sz = sizeof(double),
	},
};
struct sos_schema_s sos_long_double_array_ischema = {
	.sos = NULL,
	.flags = SOS_SCHEMA_F_INTERNAL,
	.ref_count = 1,
	.data = &sos_long_double_array_ischema.data_,
	.data_ = {
		.name = "LONG_DOUBLE_ARRAY",
		.ref_count = 1,
		.id = SOS_ISCHEMA_LONG_DOUBLE_ARRAY,
		.obj_sz = sizeof(long double),
	},
};
struct sos_schema_s sos_obj_array_ischema = {
	.sos = NULL,
	.flags = SOS_SCHEMA_F_INTERNAL,
	.ref_count = 1,
	.data = &sos_obj_array_ischema.data_,
	.data_ = {
		.name = "OBJ_ARRAY",
		.ref_count = 1,
		.id = SOS_ISCHEMA_OBJ_ARRAY,
		.obj_sz = sizeof(struct sos_obj_ref_s),
	},
};

sos_schema_t get_ischema(sos_type_t type)
{
	assert(type >= SOS_TYPE_OBJ);
	switch (type) {
	case SOS_TYPE_OBJ:
		return &sos_obj_ischema;
	case SOS_TYPE_BYTE_ARRAY:
		return &sos_byte_array_ischema;
	case SOS_TYPE_INT32_ARRAY:
		return &sos_int32_array_ischema;
	case SOS_TYPE_INT64_ARRAY:
		return &sos_int64_array_ischema;
	case SOS_TYPE_UINT32_ARRAY:
		return &sos_uint32_array_ischema;
	case SOS_TYPE_UINT64_ARRAY:
		return &sos_uint64_array_ischema;
	case SOS_TYPE_FLOAT_ARRAY:
		return &sos_float_array_ischema;
	case SOS_TYPE_DOUBLE_ARRAY:
		return &sos_double_array_ischema;
	case SOS_TYPE_LONG_DOUBLE_ARRAY:
		return &sos_long_double_array_ischema;
	case SOS_TYPE_OBJ_ARRAY:
		return &sos_obj_array_ischema;
	}
}

int sos_attr_is_ref(sos_attr_t attr)
{
	return attr->data->type >= SOS_TYPE_OBJ;
}

int sos_attr_is_array(sos_attr_t attr)
{
	return attr->data->type >= SOS_TYPE_ARRAY;
}

static sos_value_t mem_value_init(sos_value_t val, sos_attr_t attr)
{
	size_t elem_count;
	sos_schema_t schema;

	val->attr = attr;
	if (sos_attr_is_ref(attr)) {
		errno = EINVAL;
		return NULL;
	}
	val->data = &val->data_;
	return val;
}

sos_value_t sos_value_new()
{
	return calloc(1, sizeof(struct sos_value_s));
}

void sos_value_free(sos_value_t v)
{
	free(v);
}

void *sos_obj_ptr(sos_obj_t obj)
{
	sos_obj_data_t data = obj->obj->as.ptr;
	return data->data;
}

sos_value_t sos_value(sos_obj_t obj, sos_attr_t attr)
{
	sos_value_t value = sos_value_new();
	if (value)
		value = sos_value_init(value, obj, attr);
	return value;
}

sos_value_t sos_value_init(sos_value_t val, sos_obj_t obj, sos_attr_t attr)
{
	ods_obj_t ref_obj;
	sos_schema_t schema;

	if (!obj)
		return mem_value_init(val, attr);

	val->attr = attr;
	val->obj = sos_obj_get(obj);
	val->data = (sos_value_data_t)&obj->obj->as.bytes[attr->data->offset];
	if (!sos_attr_is_array(attr))
		return val;
	/* Follow the reference to the object */
	ref_obj = ods_ref_as_obj(obj->part->obj_ods, val->data->prim.ref_);
	sos_obj_put(val->obj);
	if (!ref_obj)
		return NULL;
	val->obj = __sos_init_obj(obj->sos, get_ischema(attr->data->type), ref_obj, obj->part);
	val->data = (sos_value_data_t)&SOS_OBJ(ref_obj)->data[0];
	return val;
}

size_t sos_array_count(sos_value_t val)
{
	return val->data->array.count;
}

/*
 * Allocate an ODS object of the requested size and extend the store if necessary
 * NB: The lock protects multiple threads from attempting to extend if the
 * store requires expansion
 */
ods_obj_t __sos_obj_new(ods_t ods, size_t size, pthread_mutex_t *lock)
{
	int rc;
	ods_obj_t ods_obj;
	size_t extend_size = (size < SOS_ODS_EXTEND_SZ ? SOS_ODS_EXTEND_SZ : size << 4);
	pthread_mutex_lock(lock);
	ods_obj = ods_obj_alloc(ods, size);
	if (!ods_obj) {
		int rc = ods_extend(ods, extend_size);
		if (rc)
			goto err_0;
		ods_obj = ods_obj_alloc(ods, size);
	}
 err_0:
	pthread_mutex_unlock(lock);
	return ods_obj;
}

sos_value_t sos_array_new(sos_value_t val, sos_attr_t attr, sos_obj_t obj, size_t count)
{
	ods_obj_t array_obj;
	sos_schema_t schema;
	if (!sos_attr_is_array(attr)) {
		errno = EINVAL;
		return NULL;
	}
	schema = get_ischema(attr->data->type);
	size_t size =
		sizeof(struct sos_obj_data_s)
		+ sizeof(uint32_t) /* element count */
		+ (count * schema->data->obj_sz); /* array elements */

	array_obj = __sos_obj_new(obj->part->obj_ods, size, &obj->sos->lock);
	if (!array_obj)
		goto err;

	val->attr = attr;
	val->data = (sos_value_data_t)&obj->obj->as.bytes[attr->data->offset];

	struct sos_array_s *array = (struct sos_array_s *)&SOS_OBJ(array_obj)->data[0];
	array->count = count;
	val->data->prim.ref_ = ods_obj_ref(array_obj);
	val->obj = __sos_init_obj(obj->sos, schema, array_obj, obj->part);
	if (!val->obj)
		goto err;
	val->data = (sos_value_data_t)&SOS_OBJ(array_obj)->data[0];
	return val;
 err:
	errno = ENOMEM;
	return NULL;
}

size_t sos_value_memset(sos_value_t val, void *buf, size_t buflen)
{
	void *dst;
	if (buflen > sos_value_size(val))
		buflen = sos_value_size(val);
	if (!sos_attr_is_array(val->attr))
		dst = val->data;
	else
		dst = &val->data->array.data.byte_[0];
	memcpy(dst, buf, buflen);
	return buflen;
}

void sos_value_put(sos_value_t value)
{
	if (!value)
		return;
	if (value->obj) {
		sos_obj_put(value->obj);
	} else {
		if (value->data != &value->data_)
			free(value->data);
	}
}

sos_value_t sos_value_by_name(sos_value_t value, sos_schema_t schema, sos_obj_t obj,
			      const char *name, int *attr_id)
{
	int i;
	sos_attr_t attr = sos_schema_attr_by_name(schema, name);
	if (!attr)
		return NULL;
	return sos_value_init(value, obj, attr);
}

sos_value_t sos_value_by_id(sos_value_t value, sos_obj_t obj, int attr_id)
{
	sos_attr_t attr = sos_schema_attr_by_id(obj->schema, attr_id);
	if (!attr)
		return NULL;
	return sos_value_init(value, obj, attr);
}

int sos_attr_index(sos_attr_t attr)
{
	return attr->data->indexed;
}

size_t sos_attr_size(sos_attr_t attr)
{
	return type_sizes[attr->data->type];
}

size_t sos_value_size(sos_value_t value)
{
	return value->attr->size_fn(value);
}

void *sos_value_as_key(sos_value_t value)
{
	return value->attr->key_value_fn(value);
}

char *sos_obj_attr_to_str(sos_obj_t obj, sos_attr_t attr, char *str, size_t len)
{
	struct sos_value_s v_;
	sos_value_t v = sos_value_init(&v_, obj, attr);
	if (!v)
		return NULL;
	char *s = v->attr->to_str_fn(v, str, len);
	sos_value_put(v);
	return s;
}

const char *sos_value_to_str(sos_value_t v, char *str, size_t len)
{
	return v->attr->to_str_fn(v, str, len);
}

int sos_obj_attr_from_str(sos_obj_t obj, sos_attr_t attr, const char *str, char **endptr)
{
	int rc;
	sos_value_t v;
	struct sos_value_s v_;
	if (!sos_attr_is_array(attr)) {
		v = sos_value_init(&v_, obj, attr);
		if (v) {
			rc = sos_value_from_str(v, str, endptr);
			sos_value_put(v);
		} else
			rc = EINVAL;
	} else if (sos_attr_type(attr) == SOS_TYPE_BYTE_ARRAY) {
		v = sos_value_init(&v_, obj, attr);
		size_t sz = strlen(str) + 1;
		if (v && sos_array_count(v) < sz) {
			/* Too short, delete and re-alloc */
			sos_obj_delete(v->obj);
			sos_obj_put(v->obj);
			v = NULL;
		}
		if (!v) {
			/* The array has not been allocated yet */
			v = sos_array_new(&v_, attr, obj, sz);
			if (!v)
				return ENOMEM;
			rc = sos_value_from_str(v, str, endptr);
			sos_value_put(v);
		}
	} else
		rc = EINVAL;
	return rc;
}

int sos_value_from_str(sos_value_t v, const char *str, char **endptr)
{
	return v->attr->from_str_fn(v, str, endptr);
}


/**
 * \brief Create a copy of a schema
 *
 * Create a replica of a schema that is not associated with a
 * container. This function is useful for adding a schema from one
 * container to another container.
 *
 * \param schema
 * \retval A pointer to the new schema
 * \retval NULL Insufficient resources
 */
sos_schema_t sos_schema_dup(sos_schema_t schema)
{
	sos_schema_t dup;
	sos_attr_t attr, src_attr;
	int idx;

	if (!schema)
		return NULL;
	dup = calloc(1, sizeof(*schema));
	if (!dup)
		return NULL;
	dup->ref_count = 1;
	TAILQ_INIT(&dup->attr_list);
	dup->data = &dup->data_;
	*dup->data = *schema->data;
	dup->dict = calloc(dup->data->attr_cnt, sizeof(sos_attr_t));
	if (!dup->dict)
		goto err_0;
	idx = 0;
	TAILQ_FOREACH(src_attr, &schema->attr_list, entry) {
		attr = attr_new(dup, src_attr->data->type);
		if (!attr)
			goto err_1;
		schema->dict[idx++] = attr;
		*attr->data = *src_attr->data;
		TAILQ_INSERT_TAIL(&dup->attr_list, attr, entry);
	}
	rbn_init(&dup->name_rbn, dup->data->name);
	rbn_init(&dup->id_rbn, &dup->data->id);
	return dup;
 err_1:
	free(schema->dict);
	while (!TAILQ_EMPTY(&dup->attr_list)) {
		attr = TAILQ_FIRST(&schema->attr_list);
		TAILQ_REMOVE(&dup->attr_list, attr, entry);
		free(attr->key_type);
		free(attr->idx_type);
		free(attr);
	}
 err_0:
	free(dup);
	return NULL;
}

sos_schema_t __sos_schema_init(sos_t sos, ods_obj_t schema_obj)
{
	sos_attr_t attr;
	sos_schema_t schema;
	int idx;

	if (!schema_obj)
		return NULL;
	schema = calloc(1, sizeof(*schema));
	if (!schema)
		return NULL;
	schema->ref_count = 1;
	TAILQ_INIT(&schema->attr_list);
	schema->schema_obj = schema_obj;
	schema->sos = sos;
	schema->data = schema_obj->as.ptr;
	schema->dict = calloc(schema->data->attr_cnt, sizeof(sos_attr_t));
	if (!schema->dict)
		goto err_0;
	for (idx = 0; idx < schema->data->attr_cnt; idx++) {
		attr = attr_new(schema, schema->data->attr_dict[idx].type);
		if (!attr)
			goto err_1;
		schema->dict[idx] = attr;
		attr->data = &schema->data->attr_dict[idx];
		TAILQ_INSERT_TAIL(&schema->attr_list, attr, entry);
	}
	rbn_init(&schema->name_rbn, schema->data->name);
	rbt_ins(&sos->schema_name_rbt, &schema->name_rbn);
	rbn_init(&schema->id_rbn, &schema->data->id);
	rbt_ins(&sos->schema_id_rbt, &schema->id_rbn);
	sos->schema_count++;
	return schema;
 err_1:
	free(schema->dict);
	while (!TAILQ_EMPTY(&schema->attr_list)) {
		attr = TAILQ_FIRST(&schema->attr_list);
		TAILQ_REMOVE(&schema->attr_list, attr, entry);
		free(attr->key_type);
		free(attr->idx_type);
		free(attr);
	}
	ods_obj_put(schema_obj);
 err_0:
	free(schema);
	return NULL;
}

int __sos_schema_open(sos_t sos, sos_schema_t schema, ods_obj_t part_obj)
{
	int rc;
	sos_attr_t attr;
	char tmp_path[PATH_MAX];

	TAILQ_FOREACH(attr, &schema->attr_list, entry) {
		if (!attr->data->indexed)
			continue;
		sprintf(tmp_path, "%s/%s/%s_%s_idx",
			sos->path, SOS_PART(part_obj)->name,
			schema->data->name, attr->data->name);
		sos_idx_part_t part = calloc(1, sizeof *part);
		if (!part) {
			rc = ENOMEM;
			goto err;
		}
		part->part_obj = ods_obj_get(part_obj);
	retry:
		part->index = ods_idx_open(tmp_path, sos->o_perm);
		if (!part->index) {
			rc = ods_idx_create(tmp_path, sos->o_mode,
					    attr->idx_type, attr->key_type, 5);
			if (rc)
				goto err;
			goto retry;
		}
		TAILQ_INSERT_TAIL(&attr->idx_list, part, entry);
	}
	return 0;
 err:
	return rc;
}

/**
 * \brief Find a schema in a container.
 *
 * Find the schema with the specified name. The schema returned may be
 * used to create objects of this type in the container.
 *
 * \param sos	The container handle
 * \param name	The name of the schema
 * \returns A pointer to the schema or a NULL pointer if a schema with
 * that name does not exist in the container.
 */
sos_schema_t sos_schema_by_name(sos_t sos, const char *name)
{
	sos_schema_t schema;
	struct rbn *rbn = rbt_find(&sos->schema_name_rbt, (void *)name);
	if (!rbn)
		return NULL;
	schema = container_of(rbn, struct sos_schema_s, name_rbn);
	return sos_schema_get(schema);
}

sos_schema_t sos_schema_by_id(sos_t sos, uint32_t id)
{
	sos_schema_t schema;
	struct rbn *rbn = rbt_find(&sos->schema_id_rbt, (void *)&id);
	if (!rbn)
		return NULL;
	schema = container_of(rbn, struct sos_schema_s, id_rbn);
	return sos_schema_get(schema);
}

/**
 * \brief Add a schema to a container
 *
 * A a new schema to the container. A schema with the same name must
 * not already exist in the container.
 *
 * \param sos	The container handle
 * \param schema Pointer to the new schema
 * \retval 0	The schema was successfully added
 * \retval EEXIST A schema with the same name already exists in the
 * container.
 * \retval ENOMEM Insufficient resources
 * \retval EINVAL An invalid parameter was specified.
 */
int sos_schema_add(sos_t sos, sos_schema_t schema)
{
	char tmp_path[PATH_MAX];
	size_t size, key_len;
	ods_obj_t sos_obj_ref;
	ods_obj_t schema_obj;
	ods_key_t schema_key;
	sos_attr_t attr;
	int idx, rc;
	uint32_t offset;
	ods_obj_t udata;
	int o_mode;
	struct stat sb;

	/* See if this schema is already part of a container */
	if (schema->schema_obj)
		return EBUSY;

	/* Check to see if a schema by this name is already in the container */
	if (sos_schema_by_name(sos, schema->data->name))
		return EEXIST;

	udata = ods_get_user_data(sos->schema_ods);
	if (!udata)
		return ENOMEM;

	rc = ods_stat(sos->schema_ods, &sb);
	if (rc) {
		rc = errno;
		goto err_0;
	}
	o_mode = sb.st_mode;

	/* Compute the size of the schema data */
	size = sizeof(struct sos_schema_data_s);
	size += schema->data->attr_cnt * sizeof(struct sos_attr_data_s);
	schema_obj = __sos_obj_new(sos->schema_ods, size, &sos->lock);
	if (!schema_obj) {
		rc = ENOMEM;
		goto err_0;
	}
	sos_obj_ref = __sos_obj_new(ods_idx_ods(sos->schema_idx), sizeof *sos_obj_ref, &sos->lock);
	if (!sos_obj_ref)
		goto err_1;
	SOS_OBJ_REF(sos_obj_ref)->ods_ref = 0;
	SOS_OBJ_REF(sos_obj_ref)->obj_ref = ods_obj_ref(schema_obj);

	key_len = strlen(schema->data->name) + 1;
	schema_key = ods_key_alloc(sos->schema_idx, key_len);
	if (!schema_key) {
		rc = ENOMEM;
		goto err_2;
	}
	ods_key_set(schema_key, schema->data->name, key_len);

	schema->schema_obj = schema_obj;
	schema->sos = sos;

	strcpy(SOS_SCHEMA(schema_obj)->name, schema->data->name);
	SOS_SCHEMA(schema_obj)->ref_count = 0;
	SOS_SCHEMA(schema_obj)->schema_sz = size;
	SOS_SCHEMA(schema_obj)->obj_sz = schema->data->obj_sz;
	SOS_SCHEMA(schema_obj)->attr_cnt = schema->data->attr_cnt;
	SOS_SCHEMA(schema_obj)->id = SOS_UDATA(udata)->dict_len;

	idx = 0;
	offset = 0;
	schema->dict = calloc(schema->data->attr_cnt, sizeof(sos_attr_t));
	if (!schema->dict)
		goto err_3;
	/*
	 * Iterate through the attribute definitions and add them to
	 * the schema object
	 */
	TAILQ_FOREACH(attr, &schema->attr_list, entry) {
		sos_attr_data_t attr_data = &SOS_SCHEMA(schema_obj)->attr_dict[idx];
		*attr_data = *attr->data;
		attr->data = attr_data;
		if (attr->data->indexed && !attr->key_type) {
			rc = EINVAL;
			goto err_3;
		}
		schema->dict[idx++] = attr;
		if (!attr->data->indexed)
			continue;
	}
	schema->data = schema_obj->as.ptr;
	rc = ods_idx_insert(sos->schema_idx, schema_key,
			    ods_obj_ref(sos_obj_ref));
	if (rc)
		goto err_3;
	SOS_UDATA(udata)->dict[SOS_UDATA(udata)->dict_len] = ods_obj_ref(schema_obj);
	SOS_UDATA(udata)->dict_len += 1;

	rbn_init(&schema->name_rbn, schema->data->name);
	rbt_ins(&sos->schema_name_rbt, &schema->name_rbn);

	rbn_init(&schema->id_rbn, &schema->data->id);
	rbt_ins(&sos->schema_id_rbt, &schema->id_rbn);

	sos->schema_count++;

	ods_obj_put(sos_obj_ref);
	ods_obj_put(schema_key);
	ods_obj_put(udata);
	return __sos_schema_open(sos, schema, __sos_primary_obj_part(sos)->part_obj);
 err_3:
	ods_obj_delete(schema_key);
	ods_obj_put(schema_key);
 err_2:
	ods_obj_delete(sos_obj_ref);
	ods_obj_put(sos_obj_ref);
 err_1:
	ods_obj_delete(schema_obj);
	ods_obj_put(schema_obj);
 err_0:
	ods_obj_put(udata);
	return rc;
}

sos_schema_t sos_schema_first(sos_t sos)
{
	sos_schema_t first = LIST_FIRST(&sos->schema_list);
	if (first)
		return sos_schema_get(first);
	return NULL;
}

sos_schema_t sos_schema_next(sos_schema_t schema)
{
	sos_schema_t next = LIST_NEXT(schema, entry);
	if (next)
		return sos_schema_get(next);
	return NULL;
}

/**
 * \brief Remove a schema from a container
 *
 * Remove the schema with the specified name from the container. If
 * there are any objects in the container that use the specified
 * schema, the schema cannot be deleted.
 *
 * \param sos	The container handle
 * \param name	The name of the container
 * \retval 0	The schema was successfully deleted
 * \retval EINUSE An objects exists in the container that uses the
 * specified schema
 * \retval ENOENT No schema was found with the specified name
 */
int sos_schema_delete(sos_t sos, const char *name)
{
	return ENOSYS;
}

const char *type_names[] = {
	[SOS_TYPE_INT32] = "INT32",
	[SOS_TYPE_INT64] = "INT64",
	[SOS_TYPE_UINT32] = "UINT32",
	[SOS_TYPE_UINT64] = "UINT64",
	[SOS_TYPE_FLOAT] = "FLOAT",
	[SOS_TYPE_DOUBLE] = "DOUBLE",
	[SOS_TYPE_LONG_DOUBLE] = "LONG_DOUBLE",
	[SOS_TYPE_TIMESTAMP] = "TIMESTAMP",
	[SOS_TYPE_OBJ] = "OBJ",
	[SOS_TYPE_BYTE_ARRAY] = "BYTE_ARRAY",
	[SOS_TYPE_INT32_ARRAY] = "INT32_ARRAY",
	[SOS_TYPE_INT64_ARRAY] = "INT64_ARRAY",
	[SOS_TYPE_UINT32_ARRAY] = "UINT32_ARRAY",
	[SOS_TYPE_UINT64_ARRAY] = "UINT64_ARRAY",
	[SOS_TYPE_FLOAT_ARRAY] = "FLOAT_ARRAY",
	[SOS_TYPE_DOUBLE_ARRAY] = "DOUBLE_ARRAY",
	[SOS_TYPE_LONG_DOUBLE_ARRAY] = "LONG_DOUBLE_ARRAY",
	[SOS_TYPE_OBJ_ARRAY] = "OBJ_ARRAY"
};

const char *type_name(sos_type_t t)
{
	if (t <= SOS_TYPE_LAST)
		return type_names[t];
	return "corrupted!";
}


/**
 * \brief Print the schema to a File pointer
 *
 * This convenience function formats the schema definition in YAML an
 * writes it to the specified FILE pointer. For example:
 *
 *     sos_schema_t schema = sos_container_find(sos, "Sample");
 *     sos_schema_print(schema, stdout);
 *
 * \param schema The schema handle
 * \param fp A FILE pointer
 */
void sos_schema_print(sos_schema_t schema, FILE *fp)
{
	sos_attr_t attr;
	fprintf(fp, "schema :\n");
	fprintf(fp, "    name      : %s\n", schema->data->name);
	fprintf(fp, "    schema_sz : %ld\n", schema->data->schema_sz);
	fprintf(fp, "    obj_sz    : %ld\n", schema->data->obj_sz);
	fprintf(fp, "    id        : %d\n", schema->data->id);
	TAILQ_FOREACH(attr, &schema->attr_list, entry) {
		fprintf(fp, "    -attribute : %s\n", attr->data->name);
		fprintf(fp, "        type          : %s\n", type_name(attr->data->type));
		fprintf(fp, "        idx           : %d\n", attr->data->id);
		fprintf(fp, "        indexed       : %d\n", attr->data->indexed);
		fprintf(fp, "        offset        : %ld\n", attr->data->offset);
	}
}

/** @} */