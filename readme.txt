Wasta-Backup is a simple backup GUI using rdiff-backup for version backups of
    data to an external USB device.

Wasta-Backup will auto-launch when a USB device with a previous Wasta-Backup
    on it is inserted.

Restore possibilities include restoring previous versions of existing files or
    folders as well as restoring deleted files or folders from the backup
    device. In the case of restoring previous versions of existing items, the
    current item is first renamed using the current date and time.

Additionally, a 'Restore ALL' option is available that will replace all data
    on the computer from the backup device.

The following configuration files are used by Wasta-Backup:

        - backupDirs.txt: specifies which folders to backup and other parameters
          such as if backupInclude.txt should be used and the number of backup
          versions to keep

        - backupInclude.txt: specifies which file extensions to backup, so files
          with media extensions, for example, will not be backed up
          (to save space on the backup device)

Default configuration files are stored in a user's ~/.config/wasta-backup/
folder and will be used for new backup devices.

After a backup device is used, it stores the configuration files. This allows
each backup device to have its own configuration settings to allow for
different storage capacities.

Use the 'File' menu to edit either the default or the backup device
'backupDirs.txt' configuration.

Wasta-Backup Website:
    https://www.wastalinux.org/wasta-apps/wasta-backup/
