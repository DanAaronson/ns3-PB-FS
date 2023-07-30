#!/bin/bash

maxlen=0
while IFS= read -r line; do
	len=`echo $line | wc -w`
	echo `expr $len - 2`
	if (($len - 2 > $maxlen)); then
		maxlen=`expr $len - 2`
	fi
done < "${1:-/dev/stdin}"
echo "Max is: $maxlen"
