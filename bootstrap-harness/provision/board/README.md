# Board (Mango Pi) side provisioning scripts

Run on the board from the boot prompt (!rc), after mounting the VM on /n/vm.

## format.rc

    format.rc

ERASES the filesystem: makes the plan9 partition table and reams hjfs.

New cards only; asks for "yes".

## populate.rc

    populate.rc

Starts hjfs, mounts it on /n/fs, copies the root from /n/vm.

Safe to rerun (overwrites files, never reams).
