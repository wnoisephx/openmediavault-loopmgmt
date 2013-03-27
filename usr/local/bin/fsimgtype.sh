#!/bin/sh
#
# This script will take a file (loop image) and try to determin if it's a image of a single file image
# or if it's a image of a whole disk (contains a partition table)
# returns 0 if it can't tell
# returns 1 if it is a disk image
# returns 2 if it is a filesystem image
# returns 3 if it is a CDROM/DVD image

# we need to have the image file at least
if [ $# -lt 1 ]; then
	exit 0
fi
img=$1

# does the file exist?  if not return 0
if [ ! -f $img ]; then
	exit 0
fi

# run file and get it's output
strout=`file $img`

# does it have a partion table?
tst=`echo $strout | grep partition | wc -l`
if [ $tst -ne 0 ]; then
	exit 1
fi

# is it  CDROM/DVD image?
tst=`echo $strout | grep "ISO 9660" | wc -l`
if [ $tst -ne 0 ]; then
	exit 3
fi

# does it have a filesystem?
tst=`echo $strout | grep filesystem | wc -l`
if [ $tst -ne 0 ]; then
	exit 2
fi

exit 0
