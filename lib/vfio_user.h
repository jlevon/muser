/*-
 *   BSD LICENSE
 *
 *   Copyright (c) Intel Corporation.
 *   All rights reserved.
 *
 * Authors: Changpeng Liu <changpeng.liu@nutanix.com>
 *          Thanos Makatos <thanos@nutanix.com>
 *
 *
 *   Redistribution and use in source and binary forms, with or without
 *   modification, are permitted provided that the following conditions
 *   are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *     * Neither the name of Intel Corporation nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 *   THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 *   "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 *   LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 *   A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 *   OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 *   SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 *   LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 *   DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 *   THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 *   (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 *   OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _VFIO_USER_H
#define _VFIO_USER_H

enum vfio_user_command {
	VFIO_USER_VERSION			= 1,
	VFIO_USER_DMA_MAP			= 2,
	VFIO_USER_DMA_UNMAP			= 3,
	VFIO_USER_DEVICE_GET_INFO		= 4,
	VFIO_USER_DEVICE_GET_REGION_INFO	= 5,
	VFIO_USER_DEVICE_GET_IRQ_INFO		= 6,
	VFIO_USER_DEVICE_SET_IRQS		= 7,
	VFIO_USER_REGION_READ			= 8,
	VFIO_USER_REGION_WRITE			= 9,
	VFIO_USER_DMA_READ			= 10,
	VFIO_USER_DMA_WRITE			= 11,
	VFIO_USER_VM_INTERRUPT			= 12,
	VFIO_USER_DEVICE_RESET			= 13,
	VFIO_USER_MAX,
};

enum vfio_user_message_type {
	VFIO_USER_MESSAGE_COMMAND		= 0,
	VFIO_USER_MESSAGE_REPLY			= 1,
};

#define VFIO_USER_FLAGS_NO_REPLY		(0x1)

struct vfio_user_header {
	uint16_t	msg_id;
	uint16_t	command;
	uint32_t	msg_size;
	struct {
		uint32_t	type     : 4;
		uint32_t	no_reply : 1;
		uint32_t	error    : 1;
		uint32_t	resvd    : 26;
	} flags;
	uint32_t	error_no;
} __attribute((packed));

#define VFIO_USER_DMA_REGION_MAPPABLE		(0x1)

struct vfio_user_dma_region {
	uint64_t	addr;
	uint64_t	size;
	uint64_t	offset;
	uint32_t	protections;
	uint32_t	flags;
} __attribute((packed));

struct vfio_user_region_access {
	uint64_t	offset;
	uint32_t	region;
	uint32_t	count;
	uint8_t		data[];
} __attribute((packed));

#endif
