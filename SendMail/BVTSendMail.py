#!/usr/bin/python
# -*- coding: utf-8 -*-

import email
import mimetypes
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText
from email.mime.image import MIMEImage
from xml.dom.minidom import parse, parseString
import smtplib
import re
import sys

globalTestResult = "PASS"
global testName
testName = "SKYW"

def sendEmail(authInfo, fromAdd, toAdd, subject, plainText, htmlText):

        strFrom = fromAdd
        strTo = ', '.join(toAdd)

        server = authInfo.get('server')
        user = authInfo.get('user')
        passwd = authInfo.get('password')

        if not (server and user and passwd) :
                #print 'incomplete login info, exit now'
                return

        # 设定root信息
        msgRoot = MIMEMultipart('related')
        msgRoot['Subject'] = subject
        msgRoot['From'] = strFrom
        msgRoot['To'] = strTo
        msgRoot.preamble = 'This is a multi-part message in MIME format.'

        # Encapsulate the plain and HTML versions of the message body in an
        # 'alternative' part, so message agents can decide which they want to display.
        msgAlternative = MIMEMultipart('alternative')
        msgRoot.attach(msgAlternative)

        #设定纯文本信息
        #msgText = MIMEText(plainText, 'plain', 'utf-8')
        #msgAlternative.attach(msgText)

        #设定HTML信息
        msgText = MIMEText(htmlText, 'html', 'utf-8')
        msgAlternative.attach(msgText)

       #发送邮件
        smtp = smtplib.SMTP()
       #设定调试级别，依情况而定
        #smtp.set_debuglevel(1)
        smtp.connect(server)
        #smtp.login(user, passwd)
        
        smtp.sendmail(strFrom, toAdd, msgRoot.as_string())
        smtp.quit()
        return
		
def GenerateReport():
	
	htmlReport = r"""<html>
					<head><body bgcolor="#444444"></head>
					<table width="100%" border="1" cellspacing="1" cellpadding="1" style="word-break:break-all; word-wrap:break-all;">
					<tr style="background-color: navy; color:white; font-weight: bold;">
						<td>OS Type</td><td> IP Address </td><td> Test Result </td><td> Log path</td></tr>"""
	
	dom = parse('BVTReport.cfg')
	global globalTestResult
	for os in dom.getElementsByTagName("OS"):
		osType = os.getAttribute( "name")
		ip = os.getAttribute( "ip")
		output = os.getAttribute( "output")
		testResult = GetTestResult(output)
		cellColor = "green"
		if testResult=="FAILED" :
			globalTestResult="FAILED"
			cellColor = "red"
		if testResult=="No Report" :
			output = ""
			cellColor = "yellow"
		htmlReport += '<tr style="background-color: #CCCCCC; color:white; font-weight: bold;">'
		htmlReport += '<td width="20%" style="background-color: navy; color:white;">' + osType + "</td>"
		htmlReport += '<td width="20%">' + ip + "</td>"
		htmlReport += '<td width="10%" style="color:' + cellColor + '">' + testResult + "</td>"
		htmlReport += '<td width="50%"><a href="' + output + '">' + output + "</a></td>"
		htmlReport += "</tr>"
	htmlReport += '<tr style="background-color: #CCCCCC; color:white; font-weight: bold;">'
	htmlReport +=  '<tr >' + "//9.123.251.157/AutoBVT/Logs/" + testName + ".log"  + '</tr>'
	htmlReport += "</tr>"
	htmlReport += "</table></html>"
	return htmlReport
	
def GetTestResult(output):
    try:
        f = file(output)
        while True:
                nextline = f.readline()
                if len(nextline) == 0: 
                        break
                line = nextline
        #line is the 2nd line from file bottom
        m = re.search('<!--(\d+)-->', line)
        f.close()
        if m.group(1) == "0" :
            return "PASS"
        else:
            return "FAILED"
    except:
        return "No Report"

if __name__ == '__main__' :
	authInfo = {}
	authInfo['server'] = '9.123.196.27'
	authInfo['user'] = 'ddd'
	authInfo['password'] = 'ddd'
	fromAdd = 'bvt.not.reply@cn.ibm.com'
#	toAdd = ['sujunsj@cn.ibm.com']
	toAdd = ['sujunsj@cn.ibm.com', 'duhfei@cn.ibm.com', 'yusuxu@cn.ibm.com', 'qwquwei@cn.ibm.com', 'duxiaod@cn.ibm.com', 'yeqingy@cn.ibm.com', 'yangyf@cn.ibm.com', 'yongh@cn.ibm.com', 'yushengh@cn.ibm.com', 'dinggd@cn.ibm.com', 'xiemx@cn.ibm.com', 'qinzl@cn.ibm.com']
	#global globalTestResult

	if len(sys.argv) > 1 :
			testName = sys.argv[1]
	else: 
			testName = ""
	htmlText = GenerateReport()
	subject = '[BVT] ' + globalTestResult + ": " + testName
	plainText = 'Plain text'

	sendEmail(authInfo, fromAdd, toAdd, subject, plainText, htmlText)
	print 'Send report finished!'
