/*
 * Copyright (c) 2019 Nutanix Inc. All rights reserved.
 *
 * Authors: Thanos Makatos <thanos@nutanix.com>
 *          Swapnil Ingle <swapnil.ingle@nutanix.com>
 *          Felipe Franciosi <felipe@nutanix.com>
 *
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are met:
 *      * Redistributions of source code must retain the above copyright
 *        notice, this list of conditions and the following disclaimer.
 *      * Redistributions in binary form must reproduce the above copyright
 *        notice, this list of conditions and the following disclaimer in the
 *        documentation and/or other materials provided with the distribution.
 *      * Neither the name of Nutanix nor the names of its contributors may be
 *        used to endorse or promote products derived from this software without
 *        specific prior written permission.
 *
 *  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 *  AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 *  IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 *  ARE DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
 *  DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 *  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 *  LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 *  OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 *  DAMAGE.
 *
 */

#include <dlfcn.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>

#include "mock.h"

struct mock_object {
	char *name;
	void *symbol;
	bool patched;
};

static struct mock_object *mock_objects;
static int _nr_symbols;

int mock_init(const char *filename, int nr_symbols, char *symbols[]) {

	void *handle;
	int err = 0, i;

	if (!nr_symbols) {
		err = -EINVAL;
		goto out;
	}

	handle = dlopen(filename, RTLD_LAZY);
	if (!handle) {
		fprintf(stderr, "failed to open %s: %s\n", filename, dlerror);
		err = -1;
		goto out;
	}

	mock_objects = calloc(nr_symbols, sizeof(struct mock_object));
	if (!mock_objects) {
		err = -errno;
		goto out;
	}

	for (i = 0; i < nr_symbols; i++) {
		void *symbol;
		char *s;

		dlerror();
		symbol = dlsym(handle, symbols[i]);
		s = dlerror();
		if (s) {
			err = -1;
			goto out;
		}

		/* FIXME strdup */
		mock_objects[i].name = symbols[i];
		mock_objects[i].symbol = symbol;
	}
	_nr_symbols = nr_symbols;

out:
	if (err) {
		if (handle)
			dlclose(handle);
		free(mock_objects);
	}
	return err;
}

void mock_destroy(void *handle) {
	dlclose(handle);
}

struct mock_object* mock_find(const char *name) {

	int i;

	for (i = 0; i < _nr_symbols; i++)
		if (!strcmp(name, mock_objects[i].name))
			return &mock_objects[i];
	return NULL;
}

/*
 * TODO maybe we can avoid passing the symbols to init completely, and have
 * patch do it. Also, we could pass the symbol name not as string and stringify
 * somehow.
 * Consider we want to mock function foo: we need to provid an implementation
 * of this function, so a call to patch would look like this:
 * patch("foo", foo);
 * So the string argument doesn't make sense at all.
 */
int _patch(const char *name) {

	struct mock_object *mock = mock_find(name);
	if (!mock)
		return -1;
	mock->patched = true;
	return 0;
}

bool is_patched(struct mock_object *mock) {
	assert(mock);
	return mock->patched;
}

void *real(struct mock_object *mock) {
	assert(mock);
	return mock->symbol;
}
