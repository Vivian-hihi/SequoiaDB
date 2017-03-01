#!/bin/bash
trap "" 1 2 3 24
ifconfig eth0 down
sleep $1 &
wait
ifconfig eth0 up
/etc/init.d/networking restart