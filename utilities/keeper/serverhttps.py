# This is HTTPS server.
# It designed to run commands given in POST request.
# It works only if certificate file 'serverhttps.pem' exists in this folder.
# To generate it you can use command:
"""
openssl req -new -x509 -keyout serverhttps.pem -out serverhttps.pem -days 3656 -nodes
"""
# If file does not exist, it just skips serving.

import os
import ssl
import subprocess
import sys
import threading

import cmd

import cgruutils

isRunning = False

BaseServer = None
BaseHandler = None

if sys.version_info[0] < 3:
    BaseHTTPServer = __import__('BaseHTTPServer', globals(), locals(), [])
    BaseServer = BaseHTTPServer.HTTPServer
    SimpleHTTPServer = __import__('SimpleHTTPServer', globals(), locals(), [])
    BaseHandler = SimpleHTTPServer.SimpleHTTPRequestHandler
else:
    http = __import__('http.server', globals(), locals(), [])
    BaseServer = http.server.HTTPServer
    BaseHandler = http.server.SimpleHTTPRequestHandler


class Handler(BaseHandler):
    def do_GET( self):
        fname,ext = os.path.splitext(__file__)
        fname += '.html'
        self.writeResponse( open( fname, 'rb').read())


    def do_POST( self):
        content_len = int(self.headers['content-length'])
        post_body = cgruutils.toStr( self.rfile.read(content_len))

        cmd.execute( post_body)
        #print('Executing:')
        #print( post_body)
        #subprocess.Popen( post_body, shell=True)

        self.writeResponse( b'STATUS: OK')


    def writeResponse( self, i_bytes):
        self.send_response(200)
        self.send_header("Access-Control-Allow-Origin", "*")
        self.send_header("Content-type", "text/html")
        self.end_headers()
        self.wfile.write( i_bytes)



def serve( i_port):
    global isRunning

    certificate,ext = os.path.splitext(__file__)
    certificate += '.pem'

    if os.path.isfile( certificate):
        print('Starting HTTPS server "https://localhost:%d/" with cert "%s".' % ( i_port, certificate))
    else:
        print('Certificate file "%s" not found, skipping HTTPS serving.' % certificate)
        return

    httpd = BaseServer(('localhost', i_port), Handler)
    httpd.socket = ssl.wrap_socket (httpd.socket, certfile=certificate, server_side=True)
    thread = threading.Thread(target = httpd.serve_forever)
    thread.daemon = True
    thread.start()

    isRunning = True

    return thread


if __name__ == '__main__':

    port = 44443

    if len( sys.argv) > 1:
        port = int( sys.argv[1])

    thread = serve(44443)

    if thread is not None:
        while True:
            thread.join(1)

