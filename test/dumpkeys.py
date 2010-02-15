#!/usr/bin/env python

import sys
import os

if __name__ == '__main__':
	try:
		while True:
			line = sys.stdin.readline()
			if len(line) == 0:
				break
			h = ' '.join([hex(ord(c)) for c in line.decode('utf-8')])
			print "%s ('%s')" % (h, line.encode('string_escape'))
	except KeyboardInterrupt:
			exit(0)

