#!/bin/bash

#screenmode means "reverse video mode" really

echo -ne "\033[?5h"	# enable

echo -ne "\033[7m"	# set reverse
echo "test1"
echo -ne "\033[27m" # clear reverse
echo "test2"
#echo -ne "\033[1;24r\033[1;1H\033[46m Left     File     Command     Options     Right\033[K\r\n\033[31m┌<─ ~/workspace/flterm ──────────────v>┐\033[m\r\n"

echo "test3"

#echo -ne "\033[?5l"	# disable
