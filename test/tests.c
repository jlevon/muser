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

#include <stdarg.h>
#include <stddef.h>
#include <setjmp.h>
#include <cmocka.h>
#include <string.h>
#include <stdio.h>
#include <assert.h>

#include "../lib/muser.h"
#include "../lib/libmuser_priv.h"

#include "mock.h"

/*
 * TODO if we have one binary per function we need to test then all mock
 * functions will _always_ be mocks, so we won't have to check whether it's
 * currently patched, they will always be patched.
 */
int dev_attach(const char *path) {
    struct mock_object *m = mock_find(__func__);
    assert(m);
    if (!is_patched(m))
        /*
         * XXX the function can be called as follows:
         * ((void (*)()) real(m))(path);
         * However we do need to know the return type.
         */
        return ((int (*)(const char*))real(m))(path);
    return mock();
}

lm_ctx_t *lm_ctx_create(lm_dev_info_t *dev_info) {
    struct mock_object *m = mock_find(__func__);
    assert(m);
    if (!is_patched(m))
        return ((lm_ctx_t* (*)(lm_dev_info_t*))real(m))(dev_info);
    return (lm_ctx_t*)mock();
}

static void test_lm_ctx_create(void **state __attribute__((unused))) {

    int err;
    lm_dev_info_t d = {0};
    lm_ctx_t *c;

    err = patch(dev_attach);
    assert(!err);

    /* XXX no dev_info */
    c = lm_ctx_create(NULL);
    assert_null(c);

    /* XXX dev_attach fails */
    d.uuid = "some UUID";
    will_return(dev_attach, -1);
    c = lm_ctx_create(&d);
    assert_null(c);

    /* XXX success */
    d.pvt = (void*)0xdeadbeef;
    will_return(dev_attach, 0);
    c = lm_ctx_create(&d);
    assert_non_null(c);
    assert_ptr_equal(c->pvt, d.pvt);
    //assert_equal(c->pci_info, pci_info);
}

int main(void)
{
    int err;
    char *symbols[] = {"lm_ctx_create", "dev_attach"};

    err = mock_init("libmuser.so", ARRAY_SIZE(symbols), symbols);

    if (err) {
        fprintf(stderr, "failed to initialize mocks: %d\n", err);
        exit(EXIT_FAILURE);
    }

    const struct CMUnitTest tests[] = {
        cmocka_unit_test(test_lm_ctx_create),
    };
    return cmocka_run_group_tests(tests, NULL, NULL);
}

/* ex: set tabstop=4 shiftwidth=4 softtabstop=4 expandtab: */
