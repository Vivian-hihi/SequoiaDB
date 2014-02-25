#!/bin/bash

#*******************************************************
#@function:install expect tool
#@time:2014/2/25
#
#*******************************************************

chmod a+x expect
cp expect /usr/local/bin/
cp libtcl8.4.so /usr/local/lib/
mkdir -p /usr/local/lib/tcl8.4
cp -R tcl/* /usr/local/lib/tcl8.4/ 
