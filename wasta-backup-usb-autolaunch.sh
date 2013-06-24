#!/bin/bash

# ==============================================================================
# wasta-backup-usb-autolaunch.sh
#
#   This script takes a device id as input, finds the mount point of the device
#       from df, determines the owner of the device, and finally launches
#       wasta-backup on the device owner's x display server.
#
#   2013-06-24: Initial script
#
# ==============================================================================

sleep 10s

USB_DEVICE="/dev/"$1
USB_MOUNT=$(df | grep $USB_DEVICE | awk '{print substr($0, index($0, $6))}')
USB_OWNER=$(stat -c "%U" "$USB_MOUNT")
PID_WASTA=$(pidof wasta-backup)

# only launch wasta-backup if device has a 'wasta-backup' folder and
# no other wasta-backup process running
if [ -e "$USB_MOUNT/wasta-backup" ] && [ ! $PID_WASTA ];
then
    xhost local:$USB_OWNER
    export DISPLAY=:0

    su $USB_OWNER -c "/data/8-ai-appData/qt/build-wasta-backup-Desktop_qt4-Release/wasta-backup \"$USB_MOUNT\"" &
fi

exit 0
