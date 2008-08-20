#!/usr/bin/env python

import sys
import os

def w(s):
	sys.stdout.write(s)
	sys.stdout.flush()

if __name__ == '__main__':
	w('abcdefghijklmnop <- this should be displayed on next two lines\n')

	w('abcdefgijklmnop')
	w('\033[4h\033[8Dh\033[4l\n')

	w('abcdefgggggggggg')
	w('\033[4l\033[9Dhijklmnop\033[4l\n')


	sys.exit(0)

