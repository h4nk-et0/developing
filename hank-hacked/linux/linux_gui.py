#!/usr/bin/python

from PyQt4.QtCore import *
from PyQt4.QtGui import *
import sys
import time
import thread

class Hacked(QDialog) :
	def __init__(self,parent = None) :
		super(Hacked,self).__init__(parent)

		self.text = QLabel("<font color=white size=5><b>You've been hacked!!. You're system is all over my terminal!!!!</b></font>")

		startChat = QPushButton('&want to chat?')
		startChat.setStyleSheet('QPushButton{	background-color:white;'
							'border-style:outset;'
							'border-width:2px;'
							'border-color:white;'
							'border-radius:10px;'
							'font:bold14px;'
							'min-width:8em;'
							'min-height:28px}')
		close = QPushButton("&don't care")
		close.setStyleSheet('QPushButton{	background-color:white;'
							'border-style:outset;'
							'border-width:2px;'
							'border-color:white;'
							'border-radius:10px;'
							'font:bold14px;'
							'min-width:8em;'
							'min-height:28px}')

		hup_layout = QHBoxLayout()
		hup_layout.addStretch()
		hup_layout.addWidget(self.text)
		hup_layout.addStretch()
		hup_layout.addStretch()

		hlow_layout = QHBoxLayout()
		hlow_layout.addStretch()
		hlow_layout.addWidget(startChat)
		hlow_layout.addWidget(close)

		layout = QGridLayout()
		layout.addLayout(hup_layout,0,0)
		layout.addLayout(hlow_layout,1,0)

		self.setLayout(layout)
		self.connect(close,SIGNAL('clicked()'),app.quit)
		self.connect(startChat,SIGNAL('clicked()'),chat)

		self.resize(1366,120)
		self.move(0,324)
		self.setWindowFlags(Qt.CustomizeWindowHint)
		self.setStyleSheet('background-color:green')

class chatSession(QDialog) :
	def __init__(self,parent = None) :
		super(chatSession,self).__init__(parent)

		print 'starting chat ...'

		try :
			thread.start_new_thread(self.readInput,())
		except :
			print 'error while starting the thread'
			quit()

		self.textbrowser = QTextBrowser()

		self.lineEdit = QLineEdit('type here and press enter')
		self.lineEdit.selectAll()
		self.lineEdit.setFocus()

		hup_layout = QHBoxLayout()
		hup_layout.addWidget(self.textbrowser)

		hlow_layout = QHBoxLayout()
		hlow_layout.addWidget(self.lineEdit)

		layout = QGridLayout()
		layout.addLayout(hup_layout,0,0)
		layout.addLayout(hlow_layout,1,0)

		self.connect(self.lineEdit,SIGNAL('returnPressed()'),self.updateText)
		self.setLayout(layout)
		self.resize(800,500)
		self.setWindowFlags(Qt.CustomizeWindowHint)
		self.setStyleSheet('background-color:black;'
					'background-image: url(./test.png);')

		self.lineEdit.selectAll()
		self.lineEdit.setFocus()

		self.exec_()

	def updateText(self) :
		line_text = self.lineEdit.text()
		self.textbrowser.append('<font color=red size=4><b>moriarty : %s'%(line_text))
		print '\rmoriarty : %s'%(self.lineEdit.text())
		self.lineEdit.clear()
		print 'sherlock : ',
		sys.stdout.flush()

	def readInput(self) :
		while True :
			text_readInput = raw_input('\rsherlock : ')
			self.textbrowser.append('<font color=orange size=4><b>sherlock : %s'%(text_readInput))

def chat() :
	hacked.close()
	chatsession = chatSession()

app = QApplication(sys.argv)
hacked = Hacked()
hacked.show()
app.exec_()
