#!/usr/bin/env python
# -*- coding: utf-8 -*-

import sys
import os
import fcntl

def w(s):
	sys.stdout.write(s)
	sys.stdout.flush()

#define SET_VT200_MOUSE 1000
#define SET_VT200_HIGHLIGHT_MOUSE 1001
#define SET_BTN_EVENT_MOUSE 1002
#define SET_ANY_EVENT_MOUSE 1003
#define SET_FOCUS_EVENT_MOUSE 1004
KIND='1003'

if __name__ == '__main__':
	# enable BTN_EVENT_MOUSE tracking
	os.system("stty -icanon min 1 time 0 -echo")
	w('\033[?'+KIND+'h')
	try:
		f = sys.stdin.fileno()
		while True:
			r = os.read(f, 6)
			if r[:3] == '\033[M':
				b,x,y = [ ord(x)-32 for x in r[3:6] ]

				mod = []
				if b & (1<<4) != 0:
					mod.append('ctrl')
				if b & (1<<3) != 0:
					mod.append('meta')
				if b & (1<<2) != 0:
					mod.append('shift')

				motion =  'motion' if b & (1<<5) != 0 else ''
				dir = 'press'
				btn = b&03
				if b & (1<<6):
					btn += 4
				elif btn == 3:
					dir = 'release'
					btn = -1
				elif btn == 2:
					btn = 3
				elif btn == 1:
					btn = 2
				elif btn == 0:
					btn = 1



				print "%8s %d %s %d,%d %s" % (dir, btn, motion, x,y, ','.join(mod) )
	finally:
		# disable BTN_EVENT_MOUSE tracking
		w('\033[?'+KIND+'l')
		os.system("stty -icanon min 1 time 0 echo")

