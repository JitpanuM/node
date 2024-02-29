#!/usr/bin/env bash
#
# Integration test for drop hook.
#
# Author: Steffen Vogel <post@steffenvogel.de>
# SPDX-FileCopyrightText: 2014-2023 Institute for Automation of Complex Power Systems, RWTH Aachen University
# SPDX-License-Identifier: Apache-2.0

set -e

DIR=$(mktemp -d)
pushd ${DIR}

function finish {
	popd
	rm -rf ${DIR}
}
trap finish EXIT

cat > input.dat <<EOF
1490500399.776379108(0)	0.000000	0.000000	0.000000	0.000000
1490500399.876379108(1)	0.587785	0.587785	0.587785	0.587785
1490500399.976379108(2)	0.951057	0.951057	0.951057	0.951057
1490500399.976379108(2)	0.951057	0.951057	0.951057	0.951057
1490500399.976379108(2)	0.951057	0.951057	0.951057	0.951057
1490500400.076379108(3)	0.951057	0.951057	0.951057	0.951057
1490500400.176379108(4)	0.587785	0.587785	0.587785	0.587785
1490500400.476379108(7)	-0.951057	-0.951057	-0.951057	-0.951057
1490500400.276379108(5)	0.000000	0.000000	0.000000	0.000000
1490500400.576379108(8)	-0.951057	-0.951057	-0.951057	-0.951057
1490500400.376379108(6)	-0.587785	-0.587785	-0.587785	-0.587785
1490500400.676379108(9)	-0.587785	-0.587785	-0.587785	-0.587785
EOF

cat > expect.dat <<EOF
1490500399.776379108(0)	0.000000	0.000000	0.000000	0.000000
1490500399.876379108(1)	0.587785	0.587785	0.587785	0.587785
1490500399.976379108(2)	0.951057	0.951057	0.951057	0.951057
1490500400.076379108(3)	0.951057	0.951057	0.951057	0.951057
1490500400.176379108(4)	0.587785	0.587785	0.587785	0.587785
1490500400.476379108(7)	-0.951057	-0.951057	-0.951057	-0.951057
1490500400.576379108(8)	-0.951057	-0.951057	-0.951057	-0.951057
1490500400.676379108(9)	-0.587785	-0.587785	-0.587785	-0.587785
EOF

villas hook drop < input.dat > output.dat

villas compare output.dat expect.dat
