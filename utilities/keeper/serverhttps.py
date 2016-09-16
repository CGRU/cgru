# This is HTTPS server.
# It designed to run commands given in POST request.
# Also it indexes curren directory like base python HTTPRequestHandler does.
# It works only if certificate file 'serverhttps.py.pem' exists in this folder.
# To generate it you can use command:
"""
openssl req -new -x509 -keyout serverhttps.py.pem -out serverhttps.py.pem -days 365 -nodes
"""
# If file does not exist, it just skips serving.

import os
import ssl
import subprocess
import sys
import threading

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
    def do_POST( self):
        content_len = int(self.headers.getheader('content-length', 0))
        post_body = self.rfile.read(content_len)

        print('Executing:')
        print( post_body)
        subprocess.Popen( post_body, shell=True)

        self.send_response(200)
        self.send_header("Content-type", "text/html")
        self.end_headers()
        self.wfile.write('STATUS: OK')


def serve( i_port):

    certificate,ext = os.path.splitext(__file__)
    certificate += '.pem'

    if os.path.isfile( certificate):
        print('Starting HTTPS server at port %d with cert "%s".' % ( i_port, certificate))
    else:
        print('Certificate file "%s" not found, skipping HTTPS serving.' % certificate)
        return

    httpd = BaseServer(('localhost', i_port), Handler)
    httpd.socket = ssl.wrap_socket (httpd.socket, certfile=certificate, server_side=True)
    thread = threading.Thread(target = httpd.serve_forever)
    thread.daemon = True
    thread.start()
    return thread


if __name__ == '__main__':

    port = 44443

    if len( sys.argv) > 1:
        port = int( sys.argv[1])

    thread = serve(44443)

    if thread is not None:
        while True:
            thread.join(1)

