/*
 * q_tsof.c		TSOF.
 *
 *		This program is free software; you can redistribute it and/or
 *		modify it under the terms of the GNU General Public License
 *		as published by the Free Software Foundation; either version
 *		2 of the License, or (at your option) any later version.
 *
 * Authors:	Radek Podgorny <radek@podgorny.cz>
 *
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

#include "utils.h"
#include "tc_util.h"

static void explain(void)
{
	fprintf(stderr, "Usage: ... tsof borders B1 B2...\n");
}

static int tsof_parse_opt(struct qdisc_util *qu, int argc, char **argv, struct nlmsghdr *n)
{
	int borders_mode = 0;
	int idx = 0;
	struct tc_tsof_qopt opt;
	struct rtattr *nest;
///	unsigned char mq = 0;

	while (argc > 0) {
		if (strcmp(*argv, "borders") == 0) {
			if (borders_mode) {
				fprintf(stderr, "Error: duplicate borders\n");
				return -1;
			}
			borders_mode = 1;
		} else if (strcmp(*argv, "help") == 0) {
			explain();
			return -1;
		} else {
			unsigned border;
			if (!borders_mode) {
				fprintf(stderr, "What is \"%s\"?\n", *argv);
				explain();
				return -1;
			}
			if (get_unsigned(&border, *argv, 10)) {
				fprintf(stderr, "Illegal \"borders\" element\n");
				return -1;
			}
			if (idx >= TCQ_TSOF_BANDS-1) {
				fprintf(stderr, "\"borders\" index >= TCQ_TSOF_BANDS-1=%u\n", TCQ_TSOF_BANDS-1);
				return -1;
			}
			opt.borders[idx++] = border;
		}
		argc--; argv++;
	}

	opt.bands = idx+1;

	nest = addattr_nest_compat(n, 1024, TCA_OPTIONS, &opt, sizeof(opt));
/*	if (mq)
		addattr_l(n, 1024, TCA_PRIO_MQ, NULL, 0);
*/
	addattr_nest_compat_end(n, nest);

	return 0;
}

int tsof_print_opt(struct qdisc_util *qu, FILE *f, struct rtattr *opt)
{
	int i;
	struct tc_tsof_qopt *qopt;
	// TODO: fix this -> get rid of PRIO
	struct rtattr *tb[TCA_PRIO_MAX+1];

	if (opt == NULL)
		return 0;

	if (parse_rtattr_nested_compat(tb, TCA_PRIO_MAX, opt, qopt,
					sizeof(*qopt)))
                return -1;

	fprintf(f, "borders");
	for (i=0; i<qopt->bands-1; i++)
		fprintf(f, " %d", qopt->borders[i]);
/*****
	if (tb[TCA_PRIO_MQ])
		fprintf(f, " multiqueue: %s ",
		    *(unsigned char *)RTA_DATA(tb[TCA_PRIO_MQ]) ? "on" : "off");
*/
	return 0;
}

struct qdisc_util tsof_qdisc_util = {
	.id	 	= "tsof",
	.parse_qopt	= tsof_parse_opt,
	.print_qopt	= tsof_print_opt,
};

