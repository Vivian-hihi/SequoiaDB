#!/bin/bash
pid=$(lsof -i:$1 | sed '1d' | awk '{print $2}')
if test -z $pid
then
  exit 1
fi
kill -9 $pid
echo $pid