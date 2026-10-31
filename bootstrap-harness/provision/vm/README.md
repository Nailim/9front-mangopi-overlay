# VM (Plan9) side provisioning scripts

Execute on 9front.

## build-kernel.rc

    build-kernel.rc

Fresh tests + clean kernel build -> ../build/kernel/ with MANIFEST (date, overlay commit, sha1s of kernel, libc.a, jc, jl) and SHA1SUM. Needs the overlay bound (bind.rc).


## root-export.rc

    root-export.rc

Read-only, UNAUTHENTICATED export of root.proto's sources and ../board on tcp 7777 until Del. Trusted LAN only. Stop any other export on that port first.

## root.proto

What is copied onto the card's root.
