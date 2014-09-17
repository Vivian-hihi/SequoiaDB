#!/bin/bash
# Absolute path to this script
SCRIPT=$(readlink -f "$0")
# Absolute path this script is in
SCRIPTPATH=$(dirname "$SCRIPT")
ROOTPATH=$SCRIPTPATH/..
if [ "$#" -ne 1 ] || ! [ -f "$1" ]; then
   echo "Usage: $0 listFile" >&2
   exit 1
fi

# loop through the file
while read line ;do
   SOURCEPATH=$ROOTPATH/$line
   if [[ -f $SOURCEPATH ]]; then
      echo "remove file $SOURCEPATH"
      rm -f $SOURCEPATH
   elif [[ -d $SOURCEPATH ]]; then
      echo "remove dir $SOURCEPATH"
      rm -rf $SOURCEPATH
   else
      echo "$SOURCEPATH is not a file nor dir"
      exit 1
   fi
done < $1
