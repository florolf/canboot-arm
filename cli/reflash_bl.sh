#!/bin/zsh

ID="$1"

./memload can0 $ID 0x20000200 <(cat ../ramstub/ramstub.bin ../boot/boot.bin)
cansend can0 "${ID}#3100000000020020"

