#!/bin/bash
trap "" 1 2 3 24
ifconfig eth0 down
sleep $1 &
wait
ifconfig eth0 up
service network restart
/etc/init.d/networking restart