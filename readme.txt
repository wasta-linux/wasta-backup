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

The following configurable settings are stored in a user's
    ~/.config/wasta-backup/ directory:
        - backupDirs.txt: specifies directories to backup and other parameters
          such as number of versions to keep
        - backupInclude.txt: specifies file extensions to backup (so files
          with media extensions, etc., will be politely ignored)

Wasta-Backup Website:
    https://sites.google.com/site/wastalinux/wasta-applications/wasta-backup
