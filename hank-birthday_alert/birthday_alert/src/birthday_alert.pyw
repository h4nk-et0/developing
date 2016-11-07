#!/usr/bin/python

#
#	title		-> hank-birthday_alert
#	description	-> monitors and displays the birthdays
#	author		-> hank
#

try :
	from PyQt4.QtCore import *
	from PyQt4.QtGui import *

except ImportError, error :
	print 'install PyQt4 package'
	print 'maybe python-qt4 ?'

import sys
import time

# the GUI object for displaying the content

class Alert(QDialog) :
	def __init__(self,parent = None) :
		super(Alert,self).__init__(parent)
		self.setAttribute(Qt.WA_DeleteOnClose)

		# upper layout containing the header

		header = QLabel('<font color = white size = 6><b>Birthday Alert !</b></font>')
		upperLayout = QHBoxLayout()
		upperLayout.addStretch()
		upperLayout.addWidget(header)
		upperLayout.addStretch()

		# the middle layout containing the event name and the date

		middleLayout = QHBoxLayout()
		text = '<font color = white size = 5><b>Name : ' + eventData.name + '<br />Date : '
		text += str(eventData.day) + '-' + str(eventData.month) + '-' + str(eventData.year) +'</b></font>'
		text_mid = QLabel(text)
		middleLayout.addStretch()
		middleLayout.addWidget(text_mid)
		middleLayout.addStretch()
		middleLayout.addStretch()

		# checkbox layout containing the 'Don't show this again today' checkbox

		self.checkBox = QCheckBox("&Don't show this again today")
		self.checkBox.setChecked(False)
		checkBoxLayout = QHBoxLayout()
		checkBoxLayout.addStretch()
		checkBoxLayout.addWidget(self.checkBox)

		# the lower layout containing the close button

		lowerLayout = QHBoxLayout()
		self.close = QPushButton('&Close')
		lowerLayout.addStretch()
		lowerLayout.addWidget(self.close)
		self.close.setStyleSheet('QPushButton{	background-color:blue;'
							'border-style:outset;'
							'border-width:2px;'
							'border-color:white;'
							'border-radius:10px;'
							'font:bold14px;'
							'min-width:8em;'
							'min-height:28px}')

		# the grid layout containing all the above layouts

		layout = QGridLayout()
		layout.addLayout(upperLayout,0,0)
		layout.addLayout(middleLayout,1,0)
		layout.addLayout(checkBoxLayout,2,0)
		layout.addLayout(lowerLayout,3,0)
		self.setLayout(layout)

		# various connections

		self.connect(self.checkBox,SIGNAL('toggled(bool)'),self.updateShow)
		self.connect(self.close,SIGNAL('clicked()'),self,SLOT('reject()'))
		self.resize(1366,120)
		self.move(0,324)
		self.setWindowFlags(Qt.CustomizeWindowHint)

		self.setStyleSheet('background-color:blue')
		self.exec_()

	# update the data if the 'Don't show this again today' checkbox is checked

	def updateShow(self) :
		if self.checkBox.isChecked() :
			eventData.showAgain = False
			eventData.checkAgain = False
		else :
			eventData.showAgain = True
			eventData.showAgain = True

# class for the custom exception

class Birthday_alert(Exception) :
	name = ' '

# class containing all the information about today's event

class EventData() :
	def __init__(self) :
		self.checkAgain = True
		self.showAgain = False
		self.name = str()
		self.day = int()
		self.month = int()
		self.year = int()
		self.date = QDate()
		self.date_before = QDate()

# class containing other data for the purpose of variable maintainance

class OtherData() :
	def __init__(self) :
		self.loop = str()
		self.firstTab = str()
		self.lastTab = str()
		self.currentDate = QDate()
		self.fileData = list()
		self.dbPath = str()
		self.today = True

app = QApplication(sys.argv)
eventData = EventData()
otherData = OtherData()
otherData.currentDate = QDate.currentDate()

# check for the platform

if sys.platform == "linux" or sys.platform == "linux2" :
	otherData.dbPath = '/media/darkden/my_code/projects/project_hank/hank-birthday_alert/birthday_db'

elif sys.platform == "win32" :
	print 'working on it'

# check if the folder hierarchy exists



# the main loop that checks for the birthday

while 1 :

	birthday_db = open(otherData.dbPath,'r')
	otherData.fileData = birthday_db.readlines()

	for otherData.loop in otherData.fileData :
		otherData.loop = otherData.loop.strip()
		if otherData.loop == None :
			continue
		elif len(otherData.loop) == 0:
			continue
		elif otherData.loop[0] == '#':
			continue
		elif otherData.loop[0][0] == '#' :
			continue
		otherData.firstTab = otherData.loop.find('\t')
		otherData.lastTab = otherData.loop.rfind('\t')
		eventData.name = otherData.loop[:otherData.firstTab]
		eventData.day,eventData.month = otherData.loop[otherData.lastTab + 1:].split('-')

		eventData.day = int(eventData.day)
		eventData.month = int(eventData.month)
		eventData.year = QDate.year(otherData.currentDate)

		eventData.date = QDate(eventData.year,eventData.month,eventData.day)
		eventData.date_before = QDate(eventData.year,eventData.month,eventData.day - 1)

		try :
			if eventData.date == otherData.currentDate :
				Birthday_alert.name = eventData.name
				eventData.showAgain = True
				eventData.today = True
				raise Birthday_alert

			elif eventData.date_before == otherData.currentDate :
				Birthday_alert.name = eventData.name
				eventData.showAgain = True
				eventData.today = False
				raise Birthday_alert

		except Birthday_alert, arg :
			alert = Alert()
			del alert
			time.sleep(2)

	birthday_db.close()
	eventData.checkAgain = False

	# the loop that checks for the date change every one hour. It will execute only if both the showAgain and checkAgain are false

	while not eventData.showAgain and not eventData.checkAgain :
		if otherData.currentDate != QDate.currentDate() :
			otherData.currentDate = QDate.currentDate()
			eventData.checkAgain = True
			eventData.showAgain = False
			break;
		time.sleep(3600)

	# it waits for half an hour to show the event again

	else :
		time.sleep(1800)
