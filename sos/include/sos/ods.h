/*
 * Copyright (c) 2012 Open Grid Computing, Inc. All rights reserved.
 * Copyright (c) 2012 Sandia Corporation. All rights reserved.
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

#ifndef __ODS_H
#define __ODS_H
#include <sys/queue.h>
#include <stdint.h>
#include <stdio.h>
#include <sos/ods_atomic.h>
#ifdef __cplusplus
extern "C" {
#endif

typedef struct ods_s *ods_t;
typedef uint64_t ods_ref_t;
typedef struct ods_map_s *ods_map_t;
typedef struct ods_obj_s *ods_obj_t;

/**
 * \brief Open and optionally create an ODS object store
 *
 * \param path	The path to the ODS to be opened/created
 * \param o_flags The open flags. These are the same as for \c open()
  * \retval !0	The ODS handle
 * \retval 0	An error occured opening/creating the ODS
 */
extern ods_t ods_open(const char *path, int o_flags, ...);

/**
 * \brief Acquire a pointer to the user-data for the ODS
 *
 * An ODS has storage pre-allocated for the storager of
 * meta-data. This function returns a pointer to that storage.
 *
 * \param ods	The ODS ods handle
 * \return A pointer to the user data.
 */
extern ods_obj_t _ods_get_user_data(ods_t ods);
#define ods_get_user_data(ods) ({		\
	ods_obj_t o = _ods_get_user_data(ods);	\
	if (o) {				\
		o->alloc_line = __LINE__;	\
		o->alloc_func = __func__;	\
	}					\
	o;					\
})

#define ODS_COMMIT_ASYNC	0
#define ODS_COMMIT_SYNC		1
/**
 * \brief Commit changes to stable storage
 *
 * This function initiates and optionally waits for these changes to
 * be committed to stable storage.
 *
 * The 'flags' parameter determines whether the function returns
 * immediately or waits for changes to be commited to stable
 * storage. If flags is set to ODS_COMMIT_SYNC is set, the function
 * will wait for the commit to complete before returning to the
 * caller.
 *
 * \param ods	The ODS handle
 * \param flags	The commit flags.
 */
extern void ods_commit(ods_t ods, int flags);

/**
 * \brief Close an ODS store
 *
 * Close the ODS store and flush all commits to persistent
 * storage. If the 'flags' parameter is set to ODS_COMMIT_SYNC, the
 * function waits until the changes are commited to stable storage
 * before returning.
 *
 * This function protects against releasing a NULL store.
 *
 * \param ods	The ods handle.
 * \param flags	Set to ODS_COMMIT_SYNC for a synchronous close.
 */
extern void ods_close(ods_t ods, int flags);

/**
 * \brief Allocate an object of the requested size
 *
 * Allocates space in the persistent store for an object of at least
 * \c sz bytes and initializes an in-memory object that refers to this
 * ODS object. The application should use the ods_obj_get()
 * function to add references to the object and ods_obj_put() to
 * release these references. The ODS storage for the object is freed
 * using the the ods_obj_delete() function.
 *
 * \param ods	The ODS handle
 * \param sz	The desired size
 * \return	Pointer to an object of the requested size or NULL if there
 *		is an error.
 */
extern ods_obj_t _ods_obj_alloc(ods_t ods, size_t sz);
#define ods_obj_alloc(ods, sz) ({		\
	ods_obj_t o = _ods_obj_alloc(ods, sz);	\
	if (o) {				\
		o->alloc_line = __LINE__;	\
		o->alloc_func = __func__;	\
	}					\
	o;					\
})
/**
 * \brief Allocate an object of the requested size
 *
 * Allocates space in memory for an object of at least
 * \c sz bytes and initializes an in-memory object that refers to this
 * ODS object. The application should use the ods_obj_get()
 * function to add references to the object and ods_obj_put() to
 * release these references.
 *
 * \param ods	The ODS handle
 * \param sz	The desired size
 * \return	Pointer to an object of the requested size or NULL if there
 *		is an error.
 */
extern ods_obj_t _ods_obj_malloc(ods_t ods, size_t sz);
#define ods_obj_malloc(ods, sz) ({		\
	ods_obj_t o = _ods_obj_malloc(ods, sz);	\
	if (o) {				\
		o->alloc_line = __LINE__;	\
		o->alloc_func = __func__;	\
	}					\
	o;					\
})

/**
 * \brief Free the storage for this object in the ODS
 *
 * This frees ODS resources associated with the object. The object
 * reference and pointer will be set to zero. Use the \c
 * ods_obj_put() function to release the in-memory resources for the
 * object.
 *
 * \param obj	Pointer to the object
 */
extern void ods_obj_delete(ods_obj_t obj);

/**
 * \brief Free the storage for this reference in the ODS
 *
 * This frees ODS resources associated with the object.
 *
 * \param ods	The ODS handle
 * \param ref	The ODS object reference
 */
extern void ods_ref_delete(ods_t ods, ods_ref_t ref);

/**
 * \brief Drop a reference to the object
 *
 * Decrement the reference count on the object. If the reference count
 * goes to zero, the in-memory resrouces associated with the object
 * will be freed.
 *
 * \param obj	Pointer to the object
 */
extern void ods_obj_put(ods_obj_t obj);

/**
 * \brief Extend the object store by the specified amount
 *
 * This function increases the size of the object store by the
 * specified amount.
 *
 * \param ods	The ODS handle
 * \param sz	The requsted additional size in bytes
 * \return 0	Success
 * \return ENOMEM There was insufficient storage to satisfy the request.
 */
extern int ods_extend(ods_t ods, size_t sz);

/**
 * \brief Return the size of the ODS in bytes
 *
 * \param ods	The ODS handle
 * \returns The size of the ODS in bytes
 */
extern size_t ods_size(ods_t ods);

/**
 * \brief Dump the meta data for the ODS
 *
 * This function prints information about the object store such as its
 * size and current allocated and free object locations.
 *
 * \param ods	The ODS handle
 * \param fp	The FILE* to which the information should be sent
 */
extern void ods_dump(ods_t ods, FILE *fp);

/**
 * \brief The callback function called by the ods_iter() function
 *
 * \param ods	The ODS handle
 * \param ptr	Pointer to the object
 * \param sz	The size of the object
 * \param arg	The 'arg' passed into ods_iter()
 */
typedef void (*ods_iter_fn_t)(ods_t ods, void *ptr, size_t sz, void *arg);

/**
 * \brief Iterate over all objects in the ODS
 *
 * This function iterates over all objects allocated in the ODS and
 * calls the specified 'iter_fn' for each object. See the
 * ods_iter_fn_t() for the function definition.
 *
 * \param ods		The ODS handle
 * \param iter_fn	Pointer to the function to call
 * \param arg		A void* argument that the user wants passed to
 *			the callback function.
 */
extern void ods_iter(ods_t ods, ods_iter_fn_t iter_fn, void *arg);

/*
 * Take a reference on an object
 */
ods_obj_t ods_obj_get(ods_obj_t obj);

/*
 * Put a reference on an object
 */
void ods_obj_put(ods_obj_t obj);

/*
 * Create a memory object from a persistent reference
 */
ods_obj_t _ods_ref_as_obj(ods_t ods, ods_ref_t ref);
#define ods_ref_as_obj(ods, ref) ({			\
	ods_obj_t o = _ods_ref_as_obj(ods, ref);	\
	if (o) {					\
		o->alloc_line = __LINE__;		\
		o->alloc_func = __func__;		\
	}						\
	o;						\
})


/*
 * Return an object's reference
 */
ods_ref_t ods_obj_ref(ods_obj_t obj);

/* In memory pointer to base of ods region on disk */
struct ods_obj_s {
	ods_atomic_t refcount;
	ods_t ods;
	size_t size;		/* allocated size in store */
	ods_ref_t ref;		/* persistent reference */
	union {
		void *ptr;
		int8_t *int8;
		uint8_t *uint8;
		int16_t *int16;
		uint16_t *uint16;
		int32_t *int32;
		uint32_t *uint32;
		int64_t *int64;
		uint64_t *uint64;
		char *str;
		unsigned char *bytes;
		ods_atomic_t *lock;
	} as;
	ods_map_t map;
	int alloc_line;
	const char *alloc_func;
	int put_line;
	const char *put_func;
	LIST_ENTRY(ods_obj_s) entry;
};
static inline void *ods_obj_as_ptr(ods_obj_t obj) {
	return obj->as.ptr;
}
/**
 * \brief Return the size of an object
 *
 * This function returns the size of the object pointed to by the
 * 'obj' parameter.
 *
 * \param obj	Pointer to the object
 * \return sz	The allocated size of the object in bytes
 * \return -1	The 'obj' parameter does not point to an ODS object.
 */
static inline size_t ods_obj_size(ods_obj_t obj) {
	return obj->size;
}

#define ODS_PTR(_typ_, _obj_) ((_typ_)_obj_->as.ptr)

#ifdef __cplusplus
}
#endif

#endif
