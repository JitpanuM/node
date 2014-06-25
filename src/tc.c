/**
 * Traffic control: setup interface queuing desciplines
 *
 * @author Steffen Vogel <stvogel@eonerc.rwth-aachen.de>
 * @copyright 2014, Institute for Automation of Complex Power Systems, EONERC
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdarg.h>

#include "utils.h"
#include "if.h"
#include "tc.h"

int tc_reset(struct interface *i)
{
	char cmd[128];
	snprintf(cmd, sizeof(cmd), "tc qdisc del dev %s root", i->name);

	debug(6, "system: %s", cmd);
	return system(cmd);
}

int tc_prio(struct interface *i, tc_hdl_t handle, int bands)
{
	char cmd[128];
	int len = 0;

	len += snprintf(cmd+len, sizeof(cmd)-len,
		"tc qdisc add dev %s root handle %u prio bands %u priomap",
		i->name, TC_HDL_MAJ(handle), bands);

	for (int i = 0; i < bands; i++)
		len += snprintf(cmd+len, sizeof(cmd)-len, " 0");

	debug(6, "system: %s", cmd);
	return system(cmd);
}

int tc_netem(struct interface *i, tc_hdl_t parent, struct netem *em)
{
	int len = 0;
	char cmd[256];
	len += snprintf(cmd+len, sizeof(cmd)-len,
		"tc qdisc add dev %s parent %u:%u netem",
		i->name, TC_HDL_MAJ(parent), TC_HDL_MIN(parent));

	if (em->valid & TC_NETEM_LIMIT)
		len += snprintf(cmd+len, sizeof(cmd)-len, " limit %u", em->limit);
	if (em->valid & TC_NETEM_DELAY) {
		len += snprintf(cmd+len, sizeof(cmd)-len, " delay %u", em->delay);

		if (em->valid & TC_NETEM_JITTER)
			len += snprintf(cmd+len, sizeof(cmd)-len, " %u", em->jitter);
		if (em->valid & TC_NETEM_DISTR)
			len += snprintf(cmd+len, sizeof(cmd)-len, " distribution %s", em->distribution);
	}

	if (em->valid & TC_NETEM_LOSS)
		len += snprintf(cmd+len, sizeof(cmd)-len, " loss random %u", em->loss);
	if (em->valid & TC_NETEM_DUPL)
		len += snprintf(cmd+len, sizeof(cmd)-len, " duplicate %u", em->duplicate);
	if (em->valid & TC_NETEM_CORRUPT)
		len += snprintf(cmd+len, sizeof(cmd)-len, " corrupt %u", em->corrupt);

	debug(6, "system: %s", cmd);
	return system(cmd);
}

int tc_mark(struct interface *i, tc_hdl_t flowid, int mark)
{
	char cmd[128];
	snprintf(cmd, sizeof(cmd),
		"tc filter add dev %s protocol ip handle %u fw flowid %u:%u",
		i->name, mark, TC_HDL_MAJ(flowid), TC_HDL_MIN(flowid));

	debug(6, "system: %s", cmd);
	return system(cmd);

}
