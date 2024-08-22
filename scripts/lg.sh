#!/bin/bash
# based on the instructions from edk2-platform
set -e
. ./scripts/build_common.sh
# not actually GCC5; it's GCC7 on Ubuntu 18.04.
GCC5_ARM_PREFIX=arm-linux-gnueabi- build -j$(nproc) -s -n 0 -a ARM -t GCC5 -p OMAP4430Pkg/Devices/blaze.dsc
./scripts/build_bootshim.sh
cat BootShim/BootShim.bin workspace/Build/OMAP4430Pkg/DEBUG_GCC5/FV/OMAP4430PKG_UEFI.fd > workspace/bootpayload.bin
./scripts/mkbootimg --kernel=workspace/bootpayload.bin --output=workspace/lg.img --cmdline="EDK2" --qcdt=device_specific/blaze.dtb --base=0x80008000