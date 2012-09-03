#!/bin/bash

#screenmode means "reverse video mode" really

echo -ne "\033[?5h"	# enable

echo -ne "\033[7m"	# set reverse
echo "test1"
echo -ne "\033[27m" # clear reverse
echo "test2"
#echo -ne "\033[1;24r\033[1;1H\033[46m Left     File     Command     Options     Right\033[K\r\n\033[31m┌<─ ~/workspace/flterm ──────────────v>┐\033[m\r\n"

# black on red
echo -ne "\033[30;41m"
echo "black on red"
echo -ne "\033[m"

# green on red
echo -e "\033[32;41mgreen on red\033[m"

# red on black
echo -ne "\033[31;40m"
echo "red on black"
echo -ne "\033[m"

# red on default
echo -e "\033[31;49mred on default\033[m"

# red on default (inverse)
echo -e "\033[7;31;49mred on default (inverse)\033[m"




#echo -ne "\033[?5l"	# disable
