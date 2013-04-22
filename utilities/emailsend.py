#!/usr/bin/env python

import cgruconfig

import smtplib, socket, sys

from email.mime.text import MIMEText
from email.mime.multipart import MIMEMultipart

from optparse import OptionParser
Parser = OptionParser( usage="%prog [options] message\ntype \"%prog -h\" for help", version="%prog 1.0")
Parser.add_option('-f', '--from',       dest='sender',     type = 'string', default='', help='Sender email address.')
Parser.add_option('-s', '--subject',    dest='subject',    type = 'string', default='CGRU', help='Message subject.')
Parser.add_option('-t', '--to',         dest='to',         type = 'string', action='append', default=[], help='recipients addresses.')
Parser.add_option('-m', '--smtpserver', dest='smtpserver', type = 'string', default='', help='SMTP host name.')
Parser.add_option('-e', '--encoding',   dest='encoding',   type = 'string', default='UTF-8', help='Encoding.')
Parser.add_option('-V', '--VERBOSE',    dest='verbose',    action='store_true', default=False, help='Verbose Mode.')
Parser.add_option('-D', '--DEBUG',      dest='debug',      action='store_true', default=False, help='Debug Mode.')
(Options, Args) = Parser.parse_args()

if Options.debug: Options.verbose = True

if Options.sender == '':
	Options.sender = cgruconfig.VARS['USERNAME'] + '@' + socket.gethostname() 

if Options.smtpserver == '':
	Options.smtpserver = cgruconfig.VARS['email_smtp_server']

html = """
<html>
<body>
<div style="color:#002000; padding:10px; margin:10px; border:2px solid #007000; border-radius:10px; background:#DDFFAA; box-shadow:0px 0px 10px #000000 inset;">
<div style="color:#000000; padding:10px; margin:10px; border:2px solid #007000; border-radius:10px; background:#FFFFFF; box-shadow:0px 0px 10px #000000 inset;">
%s
</div>
<a href="cgru.info" style="padding:10px;margin:10px;" target="_blank">CGRU</a>
</div>
</body>
</html>
"""
body = ' '.join( Args)
#body = '<html><body>'+body+'</body></html>'
body = html % body

msg = MIMEMultipart()
msg.attach( MIMEText( body, 'html', Options.encoding))
msg['Subject'] = Options.subject
msg['From'] = Options.sender
msg['To'] = ', '.join( Options.to)

if Options.verbose:
	print('SMTP Host: %s' % Options.smtpserver)
	print( msg)

if not Options.debug:
	smtp = smtplib.SMTP( Options.smtpserver)
	smtp.sendmail( Options.sender, Options.to, msg.as_string())
	smtp.quit()
else:
	print('DEBUG MODE. Skipping execution.')

