#!/bin/bash

# ==============================================================================
# wasta-backup: wasta-backup-postinst.sh
#
# This script is automatically run by the postinst configure step on
#   installation of wasta-backup.  It can be manually re-run, but is
#   only intended to be run at package installation.
#
# 2018-11-16 rik: initial script: need to comment out assert function for
#   exFAT filesystem compatibility
#
# ==============================================================================

# ------------------------------------------------------------------------------
# Check to ensure running as root
# ------------------------------------------------------------------------------
#   No fancy "double click" here because normal user should never need to run
if [ $(id -u) -ne 0 ]
then
	echo
	echo "You must run this script with sudo." >&2
	echo "Exiting...."
	sleep 5s
	exit 1
fi

# ------------------------------------------------------------------------------
# rdiff-backup adjustment
# ------------------------------------------------------------------------------
# rdiff-backup is crashing with an exFAT partition: it is checking if the
# target filesystem is case sensitive. exFAT is NOT case sensitive, so it goes
# into the correct code block to set that but for some reason in that block an
# assert function is called that crashes.
#
# Can comment out the assert function and then it doesn't crash??? Not sure
# why but going ahead for now

sed -i -e 's@\(assert not upper_a.lstat()\)$@\"\"\"\1 wasta: commenting out for exFAT compatibility\"\"\"@' \
    /usr/lib/python2.7/dist-packages/rdiff_backup/fs_abilities.py

# ------------------------------------------------------------------------------
# Finished
# ------------------------------------------------------------------------------
echo
echo "*** Finished with wasta-backup-postinst.sh"
echo

exit 0
