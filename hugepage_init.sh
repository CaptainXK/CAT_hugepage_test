#! /bin/bash

huge_mnt_path="/mnt/huge"

#test if dir exit
if [ ! -x $huge_mnt_path ]
then
	mkdir $huge_mnt_path
fi

#mnt dir with hugepage file system
mount -t hugetlbfs nodev "$huge_mnt_path"

#parse current hugepage size
ret_string=$(cat /proc/meminfo | grep -i hugepagesize)
# echo "$ret_string"

hugepage_sz=$(echo "$ret_string" | awk -F"[ :]+" '{print $2}' | sed 's/[[:space:]]//g')
hugepage_unit=$(echo "$ret_string" | awk -F"[ :]+" '{print $3}' | sed 's/[[:space:]]//g')

#set count of hugae page
hugepage_nb=0
echo "input number of hugepage you want:$hugepage_sz$hugepage_unit/hugepage"
read hugepage_nb

tot_hp=0
let "tot_hp=hugepage_sz*512" 

echo ${hugepage_nb} > /proc/sys/vm/nr_hugepages

#show tot hugepage memory in bytes
#echo "totally $tot_hp $hugepage_unit"

# run test.app
perf stat -e page-faults,LLC-load-misses,LLC-loads,LLC-store-misses,LLC-stores ./test.app -m $(echo $tot_hp)$(echo $hugepage_unit) -n 4 -l "2,4,6,8"

# clear hugepage fs
umount "$huge_mnt_path"

#if you want umount huge
#cat /proc/mounts to check all hugetlbfs mount information
#umount all huge dir
#then rm them
