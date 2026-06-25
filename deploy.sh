#!/usr/bin/env bash

set -e

if [ "$IP" = "" ]; then
    echo "Please provide IP address."
    exit 1;
fi

PORT=${PORT:-22}
REMOTE_USER=${REMOTE_USER:-root}
DEPLOY_PATH=/tmp/sdrv

ssh -p $PORT $REMOTE_USER@$IP "mkdir -p $DEPLOY_PATH"
if [ "$KDIR" = "" ]; then
    scp -P $PORT *.c Makefile load_dmesg.sh test_dmesg.sh $REMOTE_USER@$IP:$DEPLOY_PATH
    ssh -p $PORT $REMOTE_USER@$IP "cd $DEPLOY_PATH && make && (make unload &>/dev/null; make load)"
else
    make KDIR="$KDIR" && (
        scp -P $PORT *.c *.ko Makefile load_dmesg.sh test_dmesg.sh $REMOTE_USER@$IP:$DEPLOY_PATH
        ssh -p $PORT $REMOTE_USER@$IP "cd $DEPLOY_PATH && (make unload &>/dev/null; make load)"
    )
fi
