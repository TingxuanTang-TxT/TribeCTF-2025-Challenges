# Firmware Challenge

This is a hard level challenge in the Hardware category. This tests the user knowledge of unknown hardware system,
cryptography, and reverse engineering. This challenge uses the custom Tribe system a custom QEMU board designed for
this challenge. The user is responsible for recovering cryptographic keys and reverse engineering the system/firmware
to recover the hidden flag.

## Flag

`tribectf{W0uld_y0u_1ike_to_pl4y_a_h4rdwar3_g4m3}`

## Running

### Local

```bash
./qemu/build/qemu-system-arm-unsigned \
  -M tribe \
  -cpu cortex-r5 \
  -nographic \
  -serial mon:stdio \
  -drive file=flash.img,if=none,format=raw,id=flash0 -device loader,file=./bootloader/bin/bootloader.bin,addr=0x0 -device loader,file=./bootloader/bin/shadow.bin,addr=0x00100000
```

### Server (For competition)

```bash
docker compose up --build # To create the docker container

nc localhost 4000 # Connect to the container
```

## Solve

- Enter Solve instructions here
- Make solve scripts/writeup
