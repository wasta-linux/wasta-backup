#!/bin/bash

# ==============================================================================
# wasta-backup-udev.sh
#
#   This script launches wasta-backup-usb-autolaunch.  The reason for this
#       wrapper script is because udev must not be held up or else the USB
#       device is not ever mounted.  The mount needs to be finished to determine
#       the owner of the device in order to know the x-display-server to launch
#       wasta-backup on.
#
#   2013-06-24: Initial script
#
# ==============================================================================

/bin/bash -c "/usr/bin/wasta-backup-usb-autolaunch.sh $*" &

exit 0
