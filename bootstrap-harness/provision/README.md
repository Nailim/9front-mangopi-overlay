# Bootstraping a running system on new SD card

Temporary scaffolding: delete this folder when an installer exists.

Needs: Linux host, the 9front VM with the overlay bound, a card of 4 GB or more, serial console, USB Ethernet on the board and the same LAN as the VM.

# Structure

The READMEs and scripts are broken into folders designating where the commands are executed:

    vm      - 9front virtual machine
    host    - linux host
    board   - mango pi board

The extra build directory is there to host build artefacts from those scripts.


# Instalation steps

Steps denote step number, where and what the step needs to execute: 

    1. VM: `vm/build-kernel.rc`  -> `build/kernel/` (kernel + MANIFEST)
    2. HOST: `host/make-card.sh /dev/sdX`  (ERASES the card)
    3. HOST: mount the FAT partition, `host/stage-fat.sh /mnt/card`, unmount
    4. BOARD: boot, at the prompt type `!rc`; check `cat /net/ipifc/0/status`
    5. VM: `vm/root-export.rc` (leave running, press DEL key to stop)
    6. BOARD:
        srv -n tcp!<vm-ip>!7777 vm
        mount -n /srv/vm /n/vm
        rc /n/vm/board/format.rc       # type yes to confirm
        rc /n/vm/board/populate.rc     # ~10 minutes
    7. BOARD:
        power-cycle
    8. BOARD:   
        Optionally:
            At the prompt enter !rc
            Then run `testall` (11/11) and `negall`
            Press CTRL+D to exit console.
        At the prompt press Enter 
        At `user[glenda]:` press Enter
        Then once: `/sys/lib/newuser`
        

Expect some warnings untill everything is in place!
