#!/bin/sh

if [ $# != 2 ]
then
	echo "ERROR!!! Please provide filesdir and searchstr"
	exit 1
fi

filesdir=$1
searchstr=$2

if [ ! -d $filesdir ]
then 
	echo "ERROR!!! $filesdir is not a valid directory"
	exit 1
fi

# Get the number of unique files with the matching string
filecount=`grep -R -l "$searchstr" $filesdir | wc -l`

# Get the total number of search hits
searchcount=`grep -R -o "$searchstr" $filesdir | wc -l`

echo "The number of files are $filecount and the number of matching lines are $searchcount" 
