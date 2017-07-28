/** Main routine.
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2017, Institute for Automation of Complex Power Systems, EONERC
 * @license GNU General Public License (version 3)
 *
 * VILLASnode
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *********************************************************************************/

#include <stdlib.h>
#include <unistd.h>

#include <villas/utils.h>
#include <villas/super_node.h>
#include <villas/memory.h>
#include <villas/node.h>
#include <villas/path.h>
#include <villas/api.h>
#include <villas/web.h>
#include <villas/timing.h>
#include <villas/plugin.h>
#include <villas/kernel/kernel.h>
#include <villas/kernel/rt.h>
#include <villas/hook.h>
#include <villas/stats.h>

#ifdef ENABLE_OPAL_ASYNC
  #include <villas/nodes/opal.h>
#endif

#include "config.h"

struct super_node sn;

static void quit(int signal, siginfo_t *sinfo, void *ctx)
{
	int ret;

	if (sn.stats > 0)
		stats_print_footer(STATS_FORMAT_HUMAN);

	ret = super_node_stop(&sn);
	if (ret)
		error("Failed to stop super node");

	ret = super_node_destroy(&sn);
	if (ret)
		error("Failed to destroy super node");

	info(CLR_GRN("Goodbye!"));
	exit(EXIT_SUCCESS);
}

static void usage()
{
	printf("Usage: villas-node [CONFIG]\n");
	printf("  CONFIG is the path to an optional configuration file\n");
	printf("         if omitted, VILLASnode will start without a configuration\n");
	printf("         and wait for provisioning over the web interface.\n\n");
#ifdef ENABLE_OPAL_ASYNC
	printf("Usage: villas-node OPAL_ASYNC_SHMEM_NAME OPAL_ASYNC_SHMEM_SIZE OPAL_PRINT_SHMEM_NAME\n");
	printf("  This type of invocation is used by OPAL-RT Asynchronous processes.\n");
	printf("  See in the RT-LAB User Guide for more information.\n\n");
#endif
	printf("Supported node-types:\n");
	plugin_dump(PLUGIN_TYPE_NODE);
	printf("\n");

	printf("Supported hooks:\n");
	plugin_dump(PLUGIN_TYPE_HOOK);
	printf("\n");

	printf("Supported API commands:\n");
	plugin_dump(PLUGIN_TYPE_API);
	printf("\n");

	print_copyright();

	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[])
{
	/* Check arguments */
#ifdef ENABLE_OPAL_ASYNC
	if (argc != 4)
		usage(argv[0]);

	char *uri = "opal-shmem.conf";
#else
	if (argc == 2) {
		if (!strcmp(argv[1], "-h") ||
		    !strcmp(argv[1], "--help"))
			    usage();
	}
	else if (argc > 2)
		usage();
#endif

	info("This is VILLASnode %s (built on %s, %s)", CLR_BLD(CLR_YEL(BUILDID)),
		CLR_BLD(CLR_MAG(__DATE__)), CLR_BLD(CLR_MAG(__TIME__)));

#ifdef __linux__
	/* Checks system requirements*/
	struct version kver, reqv = { KERNEL_VERSION_MAJ, KERNEL_VERSION_MIN };
	if (kernel_get_version(&kver) == 0 && version_cmp(&kver, &reqv) < 0)
		error("Your kernel version is to old: required >= %u.%u", KERNEL_VERSION_MAJ, KERNEL_VERSION_MIN);
#endif /* __linux__ */

	signals_init(quit);
	log_init(&sn.log, V, LOG_ALL);
	log_start(&sn.log);

	super_node_init(&sn);
	super_node_parse_cli(&sn, argc, argv);
	super_node_check(&sn);
	super_node_start(&sn);

	if (sn.stats > 0)
		stats_print_header(STATS_FORMAT_HUMAN);


	if (sn.stats > 0) {
		int tfd = timerfd_create_rate(1.0 / sn.stats);

		for (;;) {
			timerfd_wait(tfd);

			for (size_t i = 0; i < list_length(&sn.paths); i++) {
				struct path *p = list_at(&sn.paths, i);

				if (p->state != STATE_STARTED)
					continue;

				for (size_t j = 0; j < list_length(&p->hooks); j++) {
					struct hook *h = list_at(&p->hooks, j);

					hook_periodic(h);
				}
			}
		}
	}
	else {
		for (;;)
			pause();
	}

	return 0;
}
