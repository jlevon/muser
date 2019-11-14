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

#include <assert.h>
#include <errno.h>
#include <stdlib.h>
#include <stdio.h>

#include "muser.h"
#include "cap.h"

static bool
cap_is_valid(uint8_t id)
{
    return id >= PCI_CAP_ID_PM && id <= PCI_CAP_ID_MAX;
}

/*
 * Returns a pointer to the next capability given a pointer to:
 * - the capabilities pointer in the header, or
 * - the next field of the previous capability.
 */
static uint8_t*
next(lm_pci_config_space_t *cfgspc, uint8_t *pos)
{
    assert(cfgspc != NULL);
    assert(pos != NULL);
    assert(pos == &cfgspc->hdr.cap || (*(pos - PCI_CAP_LIST_NEXT)) % PCI_CAP_SIZEOF == 0);

    return (uint8_t*)cfgspc + *pos;
}

/*
 * Returns a pointer to the beginning of the last capability.
 */
static uint8_t*
find_last(lm_pci_config_space_t *cfgspc)
{

    uint8_t *pos;

    assert(cfgspc != NULL);

    pos = &cfgspc->hdr.cap;
    while (pos < (uint8_t*)cfgspc + sizeof(*cfgspc) && *pos != 0) {
        pos = next(cfgspc, pos) + PCI_CAP_LIST_NEXT;
    }
    return pos - PCI_CAP_LIST_NEXT;
}

int
pci_cap_add(lm_pci_config_space_t *cfgspc, uint16_t size, cosnt char *buf)
{
    uint8_t *pos;
    uint8_t prev_size = 0;
    static const uint8_t sizes[PCI_CAP_ID_MAX + 1] = {
        [PCI_CAP_ID_PM] = PCI_PM_SIZEOF,
        [PCI_CAP_ID_EXP] = PCI_CAP_EXP_ENDPOINT_SIZEOF_V2, 
        [PCI_CAP_ID_MSIX] = PCI_CAP_MSIX_SIZEOF};

    assert(cfgspc != NULL);
    
    if (size < PCI_CAP_SIZEOF || size % PCI_CAP_SIZEOF != 0
        || !cap_is_valid(buf[PCI_CAP_LIST_ID])) {
        return -EINVAL;
    }

    assert(buf != NULL);

    pos = find_last(cfgspc);
    if (pos[PCI_CAP_LIST_NEXT] != 0) {
        return -ENOSPC;
    }
    if (pos + PCI_CAP_LIST_NEXT != &cfgspc->hdr.cap) {
        prev_size = sizes[pos[PCI_CAP_LIST_ID]];
        if (prev_size == 0) {
            return -EOPNOTSUPP;
        }
    }
    /*
     * Set the next pointer of the previous capability to point to the beginning
     * of the new one.
     */
    pos[PCI_CAP_LIST_NEXT] = (uint8_t)(pos - (uint8_t*)cfgspc) + prev_size;
    if (pos[PCI_CAP_LIST_NEXT] + size > sizeof(*cfgspc)) {
        pos[PCI_CAP_LIST_NEXT] = 0;
        return -ENOSPC;
    }
    /* pos now points to the beginning of the new capability */
    pos = next(cfgspc, pos[PCI_CAP_LIST_NEXT]);
    memcpy(pos, buf, size);
    pos[PCI_CAP_LIST_NEXT] = 0;

    return 0;
}

/* ex: set tabstop=4 shiftwidth=4 softtabstop=4 expandtab: */
