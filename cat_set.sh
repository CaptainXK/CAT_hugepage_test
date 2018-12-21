#! /bin/bash

msr_mod_str=$(lsmod | grep -i msr)
empty_str=""

echo "msr status : $msr_mod_str"

if test $msr_mod_str = $empty_str 
then
	echo "[insert msr mode]"
	modprobe msr
else
	echo "[msr exist]"
fi

echo "[set COS]"
pqos -e "llc:1=0x0000f;llc:2=0x000f0;llc:3=0x00f00;llc:4=0x0f000;llc:5=0xf0000"

echo "[bind core to COS]"
pqos -a "core:1=1;core:2=2;core:3=3;core:4=4;core:5=0"
