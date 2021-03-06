/*-
 * Copyright (c) 2011 NetApp, Inc.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY NETAPP, INC ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL NETAPP, INC OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * $FreeBSD$
 */

#include <sys/cdefs.h>
__FBSDID("$FreeBSD$");

#include <sys/types.h>

#include <machine/cpufunc.h>
#include <machine/vmm.h>
#include <machine/specialreg.h>

#include <vmmapi.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "xmsr.h"

static int cpu_vendor_intel, cpu_vendor_amd;

int
emulate_wrmsr(struct vmctx *ctx, int vcpu, uint32_t code, uint64_t val)
{

	if (cpu_vendor_intel) {
		switch (code) {
		case 0xd04:		/* Sandy Bridge uncore PMCs */
		case 0xc24:
			return (0);
		case MSR_BIOS_UPDT_TRIG:
			return (0);
		case MSR_BIOS_SIGN:
			return (0);
		default:
			break;
		}
	}
	return (-1);
}

int
emulate_rdmsr(struct vmctx *ctx, int vcpu, uint32_t num, uint64_t *val)
{
	int error = 0;

	if (cpu_vendor_intel) {
		switch (num) {
		case MSR_BIOS_SIGN:
		case MSR_IA32_PLATFORM_ID:
		case MSR_PKG_ENERGY_STATUS:
		case MSR_PP0_ENERGY_STATUS:
		case MSR_PP1_ENERGY_STATUS:
		case MSR_DRAM_ENERGY_STATUS:
			*val = 0;
			break;
		default:
			error = -1;
			break;
		}
	}
	return (error);
}

int
init_msr(void)
{
	int error;
	u_int regs[4];
	char cpu_vendor[13];

	do_cpuid(0, regs);
	((u_int *)&cpu_vendor)[0] = regs[1];
	((u_int *)&cpu_vendor)[1] = regs[3];
	((u_int *)&cpu_vendor)[2] = regs[2];
	cpu_vendor[12] = '\0';

	error = 0;
	if (strcmp(cpu_vendor, "AuthenticAMD") == 0) {
		cpu_vendor_amd = 1;
	} else if (strcmp(cpu_vendor, "GenuineIntel") == 0) {
		cpu_vendor_intel = 1;
	} else {
		fprintf(stderr, "Unknown cpu vendor \"%s\"\n", cpu_vendor);
		error = -1;
	}
	return (error);
}
