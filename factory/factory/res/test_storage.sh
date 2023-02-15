#!/bin/sh

if [ $# -lt 2 ]; then
	echo "usage: $0 <DEV_NODE> <FS_TYPE> [MOUNT_DIR] [TRY_PARTITIONS]"
	echo "e.g.: $0 /dev/block/sda vfat /tmp/xxxx true"
	exit 1
fi

DEVICE_NODE=$1
FS_TYPE=$2
MOUNT_DIR=${3:-/tmp/storage}
TRY_PARTITIONS=${4:-false}

TEST_STRING="factory_pcba_storage_test_string"


# ensure mount dir exist
if [ ! -d "$MOUNT_DIR" ]; then
	TMP_DIR="/tmp/storage"
	mkdir -p $TMP_DIR
	MOUNT_DIR=$TMP_DIR 
fi

# ensure unmounted
#umount $MOUNT_DIR > /dev/null 2>&1
            
if [ ! -b "$DEVICE_NODE" ]; then
	echo "device node [$DEVICE_NODE] not exist."
	exit 1
fi

# mount device
#mount -t $FS_TYPE $DEVICE_NODE $MOUNT_DIR
#if [ $? != 0 ]; then
if [ 1 ]; then
	if [ "$TRY_PARTITIONS" == "false" ]; then
		echo "mount $DEVICE_NODE failed"
		exit 1
	else
		# try udisk partitions
		disk_found=false
		SPLIT=""
		if [ -b "${DEVICE_NODE}p1" ];then
			SPLIT="p"
		fi

		for pr in 1 2 3 4; do
			udiskp=${DEVICE_NODE}${SPLIT}${pr}
			if [ ! -b "$udiskp" ]; then
				continue
			else
				echo "***** Trying $udiskp... "
				umount $udiskp
				echo "mount -t $FS_TYPE $udiskp $MOUNT_DIR"
				mount -t $FS_TYPE $udiskp $MOUNT_DIR
				if [ $? != 0 ];then
					echo "NG $FS_TYPE $udiskp $MOUNT_DIR"
					continue
				else
					echo "OK $FS_TYPE $udiskp $MOUNT_DIR"
					DEVICE_NODE=$udiskp
					disk_found=true
					break
				fi
			fi
		done

		if [ "$disk_found" == "false" ];then
			echo "no vaild disk found [$DEVICE_NODE]"
			exit 1
		fi
	fi
fi

# check storage capacity
#CAPACITY=`busybox df | busybox grep $MOUNT_DIR | busybox awk '{printf $2}'`
#busybox echo $CAPACITY > $RESULT_FILE

# write file
TEST_FILE="$MOUNT_DIR/flash_test"
[ -f $TEST_FILE ] && busybox rm -rf $TEST_FILE
echo $TEST_STRING > $TEST_FILE
if [ $? != 0 ]||[ ! -f $TEST_FILE ]; then
	echo "create file failed"
	exit 1
fi

# check file content
CONTENT=`busybox cat $TEST_FILE`
if [ "$CONTENT" != "$TEST_STRING" ]; then
	echo "mismatch"
	exit 1
fi

# umount
umount $MOUNT_DIR
if [ $? -ne 0 ]; then
	echo "$DEVICE_NODE unmount failed"
	exit 1
fi

# mount again and check file
mount -t $FS_TYPE $DEVICE_NODE $MOUNT_DIR
if [ $? -ne 0 ]; then
	echo "mount again failed"
	exit 1
fi

if [ ! -f $TEST_FILE ]; then
	echo "file not exist."
	exit 1
fi

CONTENT=`busybox cat $TEST_FILE`
if [ "$CONTENT" != "$TEST_STRING" ]; then
	echo "content mismatch."
	exit 1
fi

echo "$DEVICE_NODE test done."
umount $MOUNT_DIR
