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
	
	def println(self, text):
		self.write(text+'\r\n')

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

	def decom(self, enabled):
		"""6 - Origin Mode"""
		if enabled:
			self.DECSET(6);
		else:
			self.DECRST(6)

	def decawm(self, enabled):
		"""7 - Wraparound Mode"""
		if enabled:
			self.DECSET(7);
		else:
			self.DECRST(7)

	def deccolm(self, enabled):
		"""3 - 132 Column Mode"""
		if enabled:
			self.DECSET(3);
		else:
			self.DECRST(3)


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

	def CUB(self, n=None):
		"""
		CURSOR_BACKWARD [#cols=1]
		"""
		self.csi('D', [n])

	def CUF(self, n=None):
		"""
		CURSOR_FORWARD [#cols=1]
		"""
		self.csi('C', [n])

	def CUP(self, lineno=None, colno=None):
		"""
		CURSOR_POSITION [lineno=1] [colno=1]
		"""
		self.csi('H', params=[lineno, colno])

	def HVP(self, lineno=None, colno=None):
		"""
		CURSOR_POSITION [lineno=1] [colno=1]
		"""
		self.csi('f', params=[lineno, colno])

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
	
	def DECALN(self):
		"""
		DECALN—Screen Alignment Pattern
		"""
		self.write('\033#8')

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
	
	def EL(self, section=0):
		"""
		EL - Erase in Line
			0 (default)	From the cursor through the end of the line
			1	From the beginning of the line through the cursor
			2	The complete line
		"""
		self.csi('K', params=[section])

	def NEL(self):
		"""
		NEL-Next Line
		"""
		self.write('\033E')

	def holdit(self,silent=False, message=''):
		if not silent:
			self.write('Push <RETURN>')
		print 'Waiting forever... (hit CTRL-C) ' + message
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

	def tst_movements(self):
		# TODO: this is still buggy somehow.. drawn box doesn't work
		# works fine in original vttest though
		self.RIS()

		self.deccolm(False)
		width = self.w;
		
		# Compute left/right columns for a 60-column box centered in 'width'
		inner_l = (width-60)/2
		inner_r = 61+inner_l
		hlfxtra = (width-80)/2
		
		self.DECALN()
		self.CUP(9, inner_l)
		self.ED(1)
		self.CUP(18,60+hlfxtra)
		self.ED(0)
		self.EL(1)
		self.CUP(9,inner_r)
		self.EL(0)
		
		# 132: 36..97
		#  80: 10..71

		for row in range(10,16+1):
			self.CUP(row, inner_l)
			self.EL(1)
			self.CUP(row, inner_r)
			self.EL(0)

		self.CUP(17,30)
		self.EL(2)
		for col in range(1,width+1):
			self.HVP(self.h, col)
			self.write('*')
			self.HVP(1, col)
			self.write('*')
		
		self.CUP(2,2)
		for row in range(2,self.h):
			self.write('+')
			self.CUB(1)
			self.IND()
		
		self.CUP(self.h-1, width-1)
		for row in range(self.h-1, 1, -1):
			self.write('+')
			self.CUB(1)
			self.RI()
			
		self.CUP(2,1)
		for row in range(2,self.h):
			self.write('*')
			self.CUP(row,width)
			self.write('*')
			self.CUB(10)
			if row<10:
				self.NEL()
			else:
				self.write('\n')
		
		self.CUP(2,10)
		self.CUB(42+hlfxtra)
		self.CUF(2)
		for col in range(3, width-1):
			self.write('+')
			self.CUF(0)
			self.CUB(2)
			self.CUF(1)
		
		self.CUP(self.h-1, inner_r-1)
		self.CUF(42+hlfxtra)
		self.CUB(2)
		for col in range(width-2, 2, -1):
			self.write('+')
			self.CUB(1)
			self.CUF(1)
			self.CUB(0)
			self.write(chr(8))
		
		self.CUP(1,1)
		self.CUU(10)
		self.CUU(1)
		self.CUU(0)
		self.CUP(self.h, width)
		self.CUD(10)
		self.CUD(1)
		self.CUD(0)
		
		self.CUP(10, 2+inner_l)
		for row in range(10,16):
			for col in range(2+inner_l, inner_r-1):
				self.write(' ')
			self.CUD(1)
			self.CUB(58)
		
		self.CUU(5)
		self.CUF(1)
		self.write('The screen should be cleared,  and have an unbroken bor-')
		self.CUP(12, inner_l+3)
		self.write("der of *'s and +'s around the edge,   and exactly in the")
		self.CUP(13, inner_l+3)
		self.write("middle  there should be a frame of E's around this  text")
		self.CUP(14, inner_l+3)
		self.write("with  one (1) free position around it.     ")
		self.holdit()

		# DECAWM demo
		
		on_left = "IJKLMNOPQRSTUVWXYZ"
		on_right = on_left.lower()
		height = len(on_left)
		region = self.h-6
		
		self.holdit(silent=True, message='before deccolm')
		self.deccolm(False)
		width = self.w;
		
		self.holdit(silent=True, message='before text')
		self.println('Test of autowrap, mixing control and print characters.')
		self.println('The left/right margins should have letters in order:')
		
		self.holdit(silent=True)
		
		self.DECSTBM(3,region+3)
		self.decom(True)
		for i,(left,right) in enumerate(zip(on_left,on_right)):
			if i%4==0:
				# draw characters as-is, for reference
				self.CUP(region+1,1); self.write(left)
				self.CUP(region+1, width); self.write(right)
				self.write('\n')
			elif i%4==1:
				# simple wrapping
				self.CUP(region,width); self.write(on_right[i-1]+left)
				# backspace at right margin
				self.CUP(region+1,width); self.write(left+'\b '+right+'\n')
			elif i%4==2:
				# tab to right margin
				self.CUP(region+1,width); self.write(left+'\b\b\t\t'+right)
				self.CUP(region+1,2); self.write('\b'+left+'\n')
			else:
				# newline at right margin
				self.CUP(region+1,width); self.write('\n')
				self.CUP(region, 1); self.write(left)
				self.CUP(region, width); self.write(right)

		self.decom(False)
		self.DECSTBM(0, 0)
		self.CUP(self.h-2, 1)
		self.holdit()