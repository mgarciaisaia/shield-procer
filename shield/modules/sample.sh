#!/bin/bash
if [ "$(echo $@ | grep -c a)" -eq 0 ]; then
	exit 0
else
	exit 1
fi
