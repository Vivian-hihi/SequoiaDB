#!/bin/bash
# Absolute path to this script
SCRIPT=$(readlink -f "$0")
# Absolute path this script is in
SCRIPTPATH=$(dirname "$SCRIPT")
ROOTPATH=$SCRIPTPATH/..

if [ "$#" -ne 1 ]; then
   echo "Usage: $0 destinationPath" >&2
   exit 1
fi

# warning message
echo "You want to clone svn build to git"
echo "The destination directory $1 will be cleaned up"
echo "You have 10 seconds to cancel the operation by press Ctrl-C"
sleep 10

# clean up the target directory
rm -rf $1/* 2>/dev/null

# manually build SequoiaDB/engine/include/ossVer_Autogen.h
# replace WCREV to svn version
sed "s/WCREV/$(svn info | grep Revision | awk '{print $2}')/g" $ROOTPATH/misc/autogen/ossVer.tmp > $ROOTPATH/oss.tmp
# replace $ to empty string and replace ossVer_autogen.h
sed 's/\$//g' $ROOTPATH/oss.tmp > $ROOTPATH/SequoiaDB/engine/include/ossVer_Autogen.h

# copy file to new location
$SCRIPTPATH/copyFiles.sh $SCRIPTPATH/required.lst $1
if [ $? -ne 0 ]; then
   echo "Failed to copy file to $1"
   exit 1
fi

# remove all svn info
$SCRIPTPATH/removeSvn.sh $1
if [ $? -ne 0 ]; then
   echo "Failed to remove svn from $1"
   exit 1
fi

# remove comments
$SCRIPTPATH/removeCommentForSource.sh $1
if [ $? -ne 0 ]; then
   echo "Failed to remove comment from $1"
   exit 1
fi
