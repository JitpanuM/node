/** Node type: rtp
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2017-2018, Institute for Automation of Complex Power Systems, EONERC
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

#include <inttypes.h>
#include <string.h>

#include <re/re_types.h>
#include <re/re_main.h>
#include <re/re_rtp.h>
#undef ALIGN_MASK

#include <villas/plugin.h>
#include <villas/nodes/rtp.h>
#include <villas/nodes/socket.h>
#include <villas/utils.h>
#include <villas/format_type.h>

int rtp_reverse(struct node *n)
{
	struct rtp *r = (struct rtp *) n->_vd;
	struct sa tmp;

	tmp = r->local;
	r->local = r->remote;
	r->remote = tmp;

	return 0;
}

int rtp_parse(struct node *n, json_t *cfg)
{
	int ret = 0;
	struct rtp *r = (struct rtp *) n->_vd;

	const char *local, *remote;
	const char *format = "villas.binary";
	bool enable_rtcp = false;

	json_error_t err;

	ret = json_unpack_ex(cfg, &err, 0, "{ s?: s, s?: b, s: { s: s }, s: { s: s } }",
		"format", &format,
		"enable_rtcp", &enable_rtcp,
		"out",
			"address", &remote,
		"in",
			"address", &local
	);
	if (ret)
		jerror(&err, "Failed to parse configuration of node %s", node_name(n));

	/* Format */
	r->format = format_type_lookup(format);
	if(!r->format)
		error("Invalid format '%s' for node %s", format, node_name(n));

	/* Enable RTCP */ 
	r->enable_rtcp = enable_rtcp;
	if(enable_rtcp)
		error("RTCP is not implemented yet.");

	/* Remote address */
	ret = sa_decode(&r->remote, remote, strlen(remote));
	if (ret) {
		error("Failed to resolve remote address '%s' of node %s: %s",
			remote, node_name(n), strerror(ret));
	}

	/* Local address */
	ret = sa_decode(&r->local, local, strlen(local));
	if (ret) {
		error("Failed to resolve local address '%s' of node %s: %s",
			local, node_name(n), strerror(ret));
	}

	/** @todo parse * in addresses */

	return ret;
}

char * rtp_print(struct node *n)
{
	struct rtp *r = (struct rtp *) n->_vd;
	char *buf;

	char *local = socket_print_addr((struct sockaddr *) &r->local.u);
	char *remote = socket_print_addr((struct sockaddr *) &r->remote.u);

	buf = strf("format=%s, in.address=%s, out.address=%s", format_type_name(r->format), local, remote);

	free(local);
	free(remote);

	return buf;
}

int rtp_start(struct node *n)
{
	int ret;
	struct rtp *r = (struct rtp *) n->_vd;
	
	/* Initialize IO */
	ret = io_init(&r->io, r->format, &n->signals, SAMPLE_HAS_ALL & ~SAMPLE_HAS_OFFSET);
	if (ret)
		return ret;

	ret = io_check(&r->io);
	if (ret)
		return ret;

	uint16_t port = sa_port(&r->local) & ~1;
	ret = rtp_listen(&r->rs, IPPROTO_UDP, &r->local, port, port+1, r->enable_rtcp, NULL, NULL, NULL);

	/* TODO */

	return ret;
}

int rtp_stop(struct node *n)
{
	/*
	int ret;
	struct rtp *m = (struct rtp *) n->_vd;
	*/

	/* TODO */
	re_cancel();

	return -1;
}

int rtp_type_stop()
{
	/* TODO */

	return -1;
}

int rtp_read(struct node *n, struct sample *smps[], unsigned cnt, unsigned *release)
{
	/*
	struct rtp *m = (struct rtp *) n->_vd;
	int bytes;
	char data[RTP_MAX_PACKET_LEN];
	*/

    /* TODO */

	return -1;
}

int rtp_write(struct node *n, struct sample *smps[], unsigned cnt, unsigned *release)
{
	/* struct rtp *m = (struct rtp *) n->_vd; */

	/* size_t wbytes; */

	/* char data[RTP_MAX_PACKET_LEN]; */

	/* TODO */
	rtp_send(NULL, NULL, 0, 0, 0, 0, NULL);

	return cnt;
}

int rtp_fd(struct node *n)
{
	/* struct rtp *m = (struct rtp *) n->_vd; */

	error("No acces to file descriptor.");

	return -1;
}

static struct plugin p = {
	.name		= "rtp",
	.description	= "real-time transport protocol (libre)",
	.type		= PLUGIN_TYPE_NODE,
	.node		= {
		.vectorize	= 0,
		.size		= sizeof(struct rtp),
		.type.stop	= rtp_type_stop,
		.reverse	= rtp_reverse,
		.parse		= rtp_parse,
		.print		= rtp_print,
		.start		= rtp_start,
		.stop		= rtp_stop,
		.read		= rtp_read,
		.write		= rtp_write,
		.fd		= rtp_fd
	}
};

REGISTER_PLUGIN(&p)
LIST_INIT_STATIC(&p.node.instances)
