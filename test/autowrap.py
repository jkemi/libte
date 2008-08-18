#!/usr/bin/env python

import sys
import os

if __name__ == '__main__':
	if sys.argv[1] == "off":
		s = "\033[?7l"
	else:
		s = "\033[?7h"
	
	sys.stdout.write(s)
	sys.stdout.flush()
	sys.exit(0)

