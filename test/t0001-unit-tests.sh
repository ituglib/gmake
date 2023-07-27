#!/bin/sh
#
# Copyright (c) 2020 Nexbridge Inc. All Rights Reserved.
#

test_description='Module Unit Tests
'

. ./test-lib.sh

test_expect_success 'tandem_ext.h tests' '
	cat <<-EOF >expecting &&
	ToGuardian:
	(null)
	${MAKE_SUBVOL_PREFIX_GUARDIAN_LC}${this_test}
	${MAKE_SUBVOL_PREFIX_GUARDIAN_LC}${this_test}
	${MAKE_SUBVOL_PREFIX_GUARDIAN_LC}${this_test}
	
	${TEST_DISK_GRD}
	${TEST_DISK_GRD}.subvol
	${TEST_DISK_GRD}.subvol.file
	${TEST_NODE_GRD}
	${TEST_NODE_GRD}
	${TEST_NODE_GRD}.${TEST_DISK_GRD}
	${TEST_NODE_GRD}.${TEST_DISK_GRD}.subvol
	${TEST_NODE_GRD}.${TEST_DISK_GRD}.subvol.file
	ToOSS:
	(null)
	/E
	/G/${TEST_DISK_OSS}
	/G/${TEST_DISK_OSS}/subvol
	/G/${TEST_DISK_OSS}/subvol/file
	/E/${TEST_NODE_OSS}
	/E/${TEST_NODE_OSS}
	/E/${TEST_NODE_OSS}/G/${TEST_DISK_OSS}
	/E/${TEST_NODE_OSS}/G/${TEST_DISK_OSS}/subvol
	/E/${TEST_NODE_OSS}/G/${TEST_DISK_OSS}/subvol/file
	EOF
	launch_helper "${TANDEM_EXT_HELPER}" ${TEST_NODE_OSS} ${TEST_DISK_OSS} >actual &&
	test_cmp expecting actual
'

test_done
