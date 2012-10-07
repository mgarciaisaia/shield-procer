#!/bin/bash
while read pid
do
	kill $pid 2>/dev/null
done < <(ps -o pid h | grep -v $BASHPID)
