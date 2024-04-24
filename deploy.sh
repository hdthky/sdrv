#!/usr/bin/env bash

if [ "$IP" = "" ]; then
    echo "Please provide IP address."
    exit 1;
fi

PORT=${PORT:-22}
REMOTE_USER=${REMOTE_USER:-root}
DEPLOY_PATH=/tmp/sdrv

ssh -p $PORT $REMOTE_USER@$IP "mkdir -p $DEPLOY_PATH" 
if [ "$BUILD_LOCAL" = 1 ]; then
    if [ "$KDIR" = "" ]; then echo "Please provide kernal directory."; exit 1; fi
    make KDIR="$KDIR" && (
        scp -P $PORT sdrv.ko Makefile load_dmesg.sh test_dmesg.sh $REMOTE_USER@$IP:$DEPLOY_PATH
        ssh -p $PORT $REMOTE_USER@$IP "cd $DEPLOY_PATH; make unload &>/dev/null; make load"
    )
else
    scp -P $PORT sdrv.c Makefile load_dmesg.sh test_dmesg.sh $REMOTE_USER@$IP:$DEPLOY_PATH
    ssh -p $PORT $REMOTE_USER@$IP "cd $DEPLOY_PATH; make && (make unload &>/dev/null; make load)"
fi
