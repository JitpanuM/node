/** Simple copy protection
 *
 * @file
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2014-2015, Institute for Automation of Complex Power Systems, EONERC
 *   This file is part of S2SS. All Rights Reserved. Proprietary and confidential.
 *   Unauthorized copying of this file, via any medium is strictly prohibited. 
 *********************************************************************************/

#ifdef PROTECT

#include <stdio.h>
#include <string.h>
#include <time.h>
#include <sys/ptrace.h>

/** Simple copy protection.
 *
 * This function does:
 *   - checks several machine IDs for predefined values.
 *   - checks if program is debugged / traced
 *
 * This function shall be executed during program startup to
 * check if this host is allowed to execute S2SS.
 */
static inline __attribute__((always_inline)) int check_license()
{
	const struct {
		char *file, *content;
		off_t offset;
	} checks[] = {
		{ "/sys/class/dmi/id/product_uuid", "5002E503-4904-EB05-7406-0C0700080009" },
		{ "/sys/class/net/eth0/address" , "50:e5:49:eb:74:0c" },
		{ "/etc/machine-id", "0d8399d0216314f083b9ed2053a354a8" },
		{ "/dev/sda2", "\x53\xf6\xb5\xeb\x8b\x16\x46\xdc\x8d\x8f\x5b\x70\xb8\xc9\x1a\x2a", 0x468 }, /* EXT4 UUID */
	};

	if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0)
		return -1;

	if (time(NULL) > VALID_UNTIL)
		return -1;

	FILE *f;
	char buf[128];
	
	for (int i = 0; i < ARRAY_LEN(checks); i++) {
		f = fopen(checks[i].file, "r");
		if (!f)
			return -1;

		fseek(f, checks[i].offset, SEEK_SET);
		fgets(buf, sizeof(buf), f);
		fclose(f);

		if (strncmp(buf, checks[i].content, strlen(checks[i].content)))
			return -1;
	}

	return 0;
}
#else
 #define check_license() (0)
#endif
