#!/usr/bin/env python

import sys
import os

if __name__ == '__main__':
	n = int(sys.argv[1])

	s = ''
	for i in range(n):
		s += chr(ord('0') + i%10)

	sys.stdout.write(s)
	sys.stdout.flush()
	sys.exit(0)

