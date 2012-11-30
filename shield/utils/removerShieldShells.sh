#!/bin/bash
nuevo_shells=$(grep -vx /usr/bin/shield.sh /etc/shells)
echo "$nuevo_shells" > /etc/shells
