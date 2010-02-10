# -*- coding: utf-8 -*-
import os
import sys
import time

class T(object):
	def __init__(self, fifo='tpipe', w=80, h=24):
		self.t = open(fifo, 'w')
		self.w=80
		self.h=24

	def write(self, data):
		self.t.write(data)
		self.t.flush()

	def csi(self, command, params=[], intermediate=''):
		self.write('\033[' + intermediate + ';'.join([str(p) for p in params if p!=None]) +command )

	def BS(self):
		"""
		BACKSPACE
		"""
		self.write('\010')

	def RIS(self):
		"""
		RESET_TO_INITIAL_STATE
		"""
		self.write('\033c')

	def ED(self, how=0):
		"""
		ERASE_DISPLAY [how]
		where how is:
			0 - erase below
			1 - erase above
			2 - erase all
			3 - erase history
		"""
		self.csi('J', params=[how])

	def SU(self, n=1):
		"""
		SCROLL_UP [#lines=1]
		"""
		self.csi('S', params=[n])

	def SD(self, n=1):
		"""
		SCROLL_DOWN [#lines=1]
		"""
		self.csi('T', params=[n])

	def SM(self, code):
		"""
		SET_MODE what
		where what is:
			4 - Insert Mode (IRM)
			5 - Send/receive (SRM)
			6 - Automatic Newline (LNM)
		"""
		self.csi('h', params=[code])

	def DECSET(self, code):
		"""
		DEC_PRIVATE_MODE_SET what
		where what is:
			1 - Normal Cursor Keys (DECCKM)
			3 - 132 Column Mode (DECCOLM)
			5 - Screen mode (DECSCNM)
			6 - Origin mode (DECOM)
			7 - Wraparound Mode (DECAWM)
			1049 - Save cursor as in DECSC and use Alternate Screen Buffer
		"""
		self.csi('h', [code], intermediate='?')

	def RM(self, code):
		"""
		RESET_MODE
		"""
		self.csi('l', [code])

	def DECRST(self, code):
		"""
		DEC_PRIVATE_RESET_MODE
		"""
		self.csi('l', [code], intermediate='?')


	def DECSTBM(self, top=None, bottom=None):
		"""
		SET_SCROLLING_REGION [top=min] [bottom=max]
		"""
		self.csi('r', params=[top, bottom])

	def DECSTR(self):
		"""
		SOFT_TERMINAL_RESET
		"""
		self.csi('p', intermediate='!')

	def DECSC(self):
		"""
		SAVE_CURSOR
		"""
		self.csi('s')

	def DECRC(self):
		"""
		RESTORE_CURSOR
		"""
		self.csi('u')

	def CUU(self, n=1):
		"""
		CURSOR_UP [#lines=1]
		"""
		self.csi('A', [n])

	def CUD(self, n=None):
		"""
		CURSOR_DOWN [#lines=1]
		"""
		self.csi('B', [n])

	def CUP(self, lineno=None, colno=None):
		"""
		CURSOR_POSITION [lineno=1] [colno=1]
		"""
		self.csi('H', params=[lineno, colno])

	def RI(self):
		"""
		REVERSE_INDEX
		"""
		self.write('\033M')

	def IND(self):
		"""
		INDEX_DOWN
		"""
		self.write('\033D')

	def ICH(self, nchars=None):
		"""
		INSERT_CHARACTER [#chars=1]
		"""
		self.csi('@', params=[nchars])

	def DCH(self, nchars=None):
		"""
		DELETE_CHARACTERS [#nchars=1]
		"""
		self.csi('P', params=[nchars])

	def VPA(self, lineno=None):
		"""
		VERTICAL_LINE_POSITION_ABSOLUTE [lineno=1]
		"""
		self.csi('d', params=[lineno])

	def CHA(self, colno=None):
		"""
		CURSOR_HORIZONTAL_ABSOLUTE [colno=1]
		"""
		self.csi('G', params=[colno])

	def RI(self):
		"""
		REVERSE-INDEX
		"""
		self.csi('M')


	def holdit(self,silent=False):
		if not silent:
			self.write('Push <RETURN>')
		print 'Waiting forever... (hit CTRL-C)'
		try:
			while True:
				time.sleep(3)
		except:
			pass
		if not silent:
			self.write('\n')

	def do_scrolling(self):
		# vttest, main.c, do_scrolling()

		self.RIS()

		max_lines = self.h
		self.ED(2)
		self.DECSET(6)	# Origin mode (relative), DECOM

		for passs in [True, False]:
			if passs:
				first = max_lines / 2
				last = first + 1
			else:
				first = 1
				last = self.h

			self.DECSTBM(first, last)
			self.ED(2)
			for down in [False, True]:
				if down:
					self.CUU(max_lines)
				else:
					self.CUD(max_lines)
				for i in range(1, max_lines+5+1):
					self.write(
						'%s scroll %s region [%d..%d] size %d Line %d\n' %
						('Soft', 'down' if down else 'up',
							first, last, last-first+1, i
						)
					)
					if down:
						self.RI()
						self.RI()
				self.holdit()


	def origin_test(self):
		self.RIS()

		self.ED(2)
		self.DECSET(6)	# Set Origin mode (relative), DECOM

		self.DECSTBM(self.h-1,self.h)
		self.write('\nOrigin mode test. This line should be at the bottom of the screen.')
		self.CUP(1,1)
		self.write('This line should be the one above the bottom of the screen. ')
		self.holdit()

		self.ED(2)
		self.DECRST(6)	# Reset Origin mode (absolute)
		self.CUP(self.h,1)
		self.write('Origin mode test. This line should be at the bottom of the screen.')
		self.CUP(1,1)
		self.write('This line should be at the top of the screen. ')
		self.holdit()
		self.DECSTBM(1, self.h)


	def drawCrap(self):
		#self.csi('l', [3], intermediate='?')
		#self.DECSTR()
		self.RIS()
		self.DECSTBM(12,14)
		for y in range(self.h):
			for x in range(self.w):
				if (y+x) % 2 == 1:
					self.write( str(y)[-1] )
				else:
					self.write('-')

	def jkTestScroll(self):
		self.RIS()
		self.DECSTBM(3,8)
		self.CUP(1,1)
		for i in range(10):
			self.write('X')
			self.IND()

		self.CUP(10,1)
		for i in range(10):
			self.write('A')
			self.IND()

		self.holdit()

		self.RIS()
		self.DECSTBM(1,20)
		self.CUP(1,1)
		for i in range(10):
			self.write('B')
			self.IND()

		self.CUP(10,1)
		for i in range(25):
			self.write('C')
			self.IND()

		self.holdit()

	def jkTestICH_BS(self):
		self.RIS()

		for c in 'ABCDEF':
			self.write(c)
			self.BS()
			self.ICH(2)
			self.holdit(True)
#
		self.CUP(2,1)
		self.holdit()

	def jkTestDCH(self):
		self.RIS()

		self.write('AB')
		for i in range(7):
			self.write('A')
		self.write('*')
		self.CUP(1,2)
		self.DCH(78)
		self.CUP(2,1)
		self.holdit()
