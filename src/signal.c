/** Generate random packages on stdout.
 *
 * @file
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
 *
 * @addtogroup tools Test and debug tools
 * @{
 **********************************************************************************/

#include <unistd.h>
#include <math.h>
#include <string.h>

#include <villas/utils.h>
#include <villas/sample.h>
#include <villas/io/villas.h>
#include <villas/timing.h>
#include <villas/node.h>
#include <villas/plugin.h>
#include <villas/nodes/signal.h>

/* Some default values */
struct node n;
struct log l;
struct io io;

struct sample *t;

void usage()
{
	printf("Usage: villas-signal [OPTIONS] SIGNAL\n");
	printf("  SIGNAL   is on of the following signal types:\n");
	printf("    mixed\n");
	printf("    random\n");
	printf("    sine\n");
	printf("    triangle\n");
	printf("    square\n");
	printf("    ramp\n");
	printf("\n");
	printf("  OPTIONS is one or more of the following options:\n");
	printf("    -d LVL  set debug level\n");
	printf("    -f FMT  set the format\n");
	printf("    -v NUM  specifies how many values a message should contain\n");
	printf("    -r HZ   how many messages per second\n");
	printf("    -n      non real-time mode. do not throttle output.\n");
	printf("    -F HZ   the frequency of the signal\n");
	printf("    -a FLT  the amplitude\n");
	printf("    -D FLT  the standard deviation for 'random' signals\n");
	printf("    -l NUM  only send LIMIT messages and stop\n\n");

	print_copyright();
}

static void quit(int signal, siginfo_t *sinfo, void *ctx)
{
	int ret;

	ret = node_stop(&n);
	if (ret)
		error("Failed to stop node");

	ret = node_destroy(&n);
	if (ret)
		error("Failed to destroy node");

	ret = io_close(&io);
	if (ret)
		error("Failed to close output");

	ret = io_destroy(&io);
	if (ret)
		error("Failed to destroy output");

	ret = log_stop(&l);
	if (ret)
		error("Failed to stop log");

	free(t);

	info(CLR_GRN("Goodbye!"));
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[])
{
	int ret;
	struct plugin *p;

	char *format = "villas"; /** @todo hardcoded for now */

	ret = log_init(&l, l.level, LOG_ALL);
	if (ret)
		error("Failed to initialize log");

	ret = log_start(&l);
	if (ret)
		error("Failed to start log");

	ret = signals_init(quit);
	if (ret)
		error("Failed to intialize signals");

	p = plugin_lookup(PLUGIN_TYPE_NODE, "signal");
	if (!p)
		error("Signal generation is not supported.");

	ret = node_init(&n, &p->node);
	if (ret)
		error("Failed to initialize node");

	p = plugin_lookup(PLUGIN_TYPE_IO, format);
	if (!p)
		error("Invalid output format '%s'", format);

	ret = io_init(&io, &p->io, IO_FLUSH | (IO_FORMAT_ALL & ~IO_FORMAT_OFFSET));
	if (ret)
		error("Failed to initialize output");

	ret = io_open(&io, NULL);
	if (ret)
		error("Failed to open output");

	ret = node_parse_cli(&n, argc, argv);
	if (ret)
		error("Failed to parse command line options");

	ret = node_check(&n);
	if (ret)
		error("Failed to verify node configuration");

	/* Allocate memory for message buffer */
	struct signal *s = n._vd;

	t = alloc(SAMPLE_LEN(s->values));

	t->capacity = s->values;

	ret = node_start(&n);
	if (ret)
		serror("Failed to start node");

	for (;;) {
		node_read(&n, &t, 1);
		io_print(&io, &t, 1);
	}

	return 0;
}

/** @} */
