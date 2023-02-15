#!/bin/sh

if [ $# -lt 2 ]; then
	echo "usage: $0 <DEV> <ATE>"
	echo "e.g.: $0 can0 can1"
	exit 1
fi

DEVICE=$1
ATE=$2

TIMEOUT=2000

TXLOGFILE="/home/.factory/cantx.txt"
DEV_TXID="123"
DEV_TXFRAME="1122334455667788"

RXLOGFILE="/home/.factory/canrx.txt"
ATE_TXID="321"
ATE_TXFRAME="1234567890987654"

RESULT=0
for retry in 1 2 3
do
	rm -rf $TXLOGFILE $RXLOGFILE

	ps -aux | grep candump | head -1 | awk '{print $2}' | xargs kill -9

	#check tx
	candump $ATE -n 1 -T $TIMEOUT > $TXLOGFILE &
	cansend $DEVICE $DEV_TXID#$DEV_TXFRAME

	ATE_RXID=$(cat $TXLOGFILE | awk '{print $2}')
	ATE_RXFRAME=$(cat $TXLOGFILE | awk '{print $4$5$6$7$8$9$10$11}')

	if [ "$DEV_TXID" != "$ATE_RXID" -o "$DEV_TXFRAME" != "$ATE_RXFRAME" ];then
		echo "tx mismatch"
		echo "$DEV_TXID $ATE_RXID"
		echo "$DEV_TXFRAME $ATE_RXFRAME"
		if [ $retry -eq 3 ]
		then
		RESULT=1
		exit 1
		fi
		continue
	fi

	#check rx
	candump $DEVICE -n 1 -T $TIMEOUT > $RXLOGFILE &
	cansend $ATE $ATE_TXID#$ATE_TXFRAME

	DEV_RXID=$(cat $RXLOGFILE | awk '{print $2}')
	DEV_RXFRAME=$(cat $RXLOGFILE | awk '{print $4$5$6$7$8$9$10$11}')

	if [ "$DEV_RXID" != "$ATE_TXID" -o "$DEV_RXFRAME" != "$ATE_TXFRAME" ];then
		echo "rx mismatch"
		if [ $retry -eq 3 ]
		then
		RESULT=1
		exit 1
		fi
		continue
	fi


	RESULT=0
done

echo "$DEVICE test done $RESULT."
exit 0
