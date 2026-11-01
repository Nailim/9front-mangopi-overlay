# Bootstraping a running system on new SD card

Temporary scaffolding: delete this folder when an installer exists.

Needs: Linux host, the 9front VM with exposed and host mounted project folder, a card of 4 GB or more, serial console, USB Ethernet on the board and the same LAN as the VM.

# Structure

The READMEs and scripts are broken into folders designating where the commands are executed:

    vm      - 9front virtual machine
    host    - linux host
    board   - mango pi board

The extra build directory is there to host build artefacts from those scripts.

## Where each step runs

The overlay repo lives on the 9front VM. The Linux host sees it through a
FUSE (9P) mount of the VM's files, mounted as your user (not root):

    Example:

    # assuming VM has port 6666 forwarded

    VM: aux/listen1 -tv tcp!*!6666 /bin/exportfs -r /usr/glenda/9front-mangopi-overlay/

    HOST: 9pfs localhost -p 6666 ./mnt_9front_riscv_mangopi_overlay/

Also the vm is expected to have port 7777 forwarded for the root-expose.rc to share the root.proto and board to mount it.

- `vm/*.rc` run in the VM's shell, in this directory as seen by the VM.
- `host/*.sh` run on the host **inside the mounted copy of this directory**,
  with `sh host/<script>` (see below), and need the VM's build output
  (`build/kernel/9sun20i`), which appears there once `vm/build-kernel.rc` has
  run in the VM.
- `board/*.rc` run on the board, served by the VM (step 6).

If `host/stage-fat.sh` says the kernel is missing: the overlay is not mounted,
or the mount shows a stale listing. Run `vm/build-kernel.rc` in the VM, then
remount (or `ls build/kernel` on the host to refresh), and retry.

`sudo` cannot run scripts from the mount (root cannot read a FUSE mount);
run `sh host/<script>` as yourself, the scripts call `sudo` for the card.


# Instalation steps

Steps denote step number, where and what the step needs to execute: 

    0. VM: go to `bootstrap-harness/provision` sub-folder of the project
    1. VM: `vm/build-kernel.rc`  -> `build/kernel/` (kernel + MANIFEST)
    2. VM: `vm/root-export.rc` (leave running, press DEL key to stop)
    3. HOST: `host/make-card.sh /dev/sdX`  (ERASES the card)
    4. HOST: mount the FAT partition, `host/stage-fat.sh /mnt/card`, unmount
    5. BOARD: boot, at the prompt type `!rc`, then check the network:

        cat /net/ipifc/*/status

        Expect two interfaces: loopback (`device /dev/null`, `127.0.0.1`) and the USB Ethernet (`device /net/etherU...`, an address like `192.168.0.x`). The hook (`sun20i.rc`) configures both at boot, after `nusbrc`.

        If the Ethernet is missing (hub plugged in late, or enumeration was still running), bring it up by hand:

        nusbrc
        ls /net                              # note the etherU... name
        ip/ipconfig ether /net/etherU<id>    # DHCP
        cat /net/ipifc/*/status
    6. BOARD:
        srv -n tcp!<vm-ip>!7777 vm
        mount -n /srv/vm /n/vm
        rc /n/vm/board/format.rc       # type yes to confirm
        rc /n/vm/board/populate.rc     # ~10 minutes
    7. BOARD:
        power-cycle
        Optionally:
            At the prompt enter !rc
            Then run `testall` (11/11) and `negall`
            Press CTRL+D to exit console.
        At the prompt press Enter
        Set up auth:
            authid: glenda;
            authdom: mangopi;
            secstore key: leave empty;
            password: choose one;
            enable legacy p9sk1: no;
        Then set up user:
            rc /sys/lib/newuser
    8. BOARD:   
        power-cycle
        At the prompt press Enter 
        At `user[glenda]:` press Enter
    9. BOARD:
        Set up remove conectivity:
            ps | grep -e keyfs -e listen # keyfs must be there
            auth/changeuser glenda
                password: choose the same as before
                assign new Inferno/POP secret: n
                assign new Inferno/POP secret: never
                ...: leave blank
    10. REMOTE
        Connect from host (Linux):
            drawterm -h <board_ip> -a <board_ip> -u glenda
        Connect from vm (plan9):
            echo 'key proto=dp9ik dom=mangopi user=glenda !password=<the password>' >/mnt/factotum/ctl
            rcpu -h <board_ip> -u glenda


Expect some warnings untill everything is in place!
