#!/bin/sh

# deciphers command sequences into readable lines
# ex. usage:
#  aptitude | tee raw.txt ^C
#  cat raw.txt decipher.sh > formatted.txt
perl -pi -e 's/(\033.{0,1}\[[0-9;]*.)/\1\n/g'

