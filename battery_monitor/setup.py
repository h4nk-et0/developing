#!/usr/bin/python

# must be root to run this setup.
# checking for root

import subprocess

try :
	import PyQt
except :
	subprocess.Popen("sudo apt-get install python-qt4").wait()
else :
	print "[+] all dependencies already installed. nothing to do."

subprocess.Popen().wait()
