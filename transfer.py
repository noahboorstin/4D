#!/usr/bin/env python

import SimpleHTTPServer
import SocketServer
import thread
import os
import sys
import socket

if os.name != "nt":
    import fcntl
    import struct

    def get_interface_ip(ifname):
        s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
        return socket.inet_ntoa(fcntl.ioctl(s.fileno(), 0x8915, struct.pack('256s',
                                ifname[:15]))[20:24])

ip = socket.gethostbyname(socket.gethostname())
if ip.startswith("127.") and os.name != "nt":
    interfaces = [
        "eth0",
        "eth1",
        "eth2",
        "wlan0",
        "wlan1",
        "wifi0",
        "ath0",
        "ath1",
        "ppp0",
        ]
    for ifname in interfaces:
        try:
            ip = get_interface_ip(ifname)
            break
        except IOError:
            pass
print(ip)

folder = '/3ds/'+os.path.basename(os.path.dirname(os.path.abspath('__file__')))+'/'
files = os.listdir('.'+folder)

class MyHandler(SimpleHTTPServer.SimpleHTTPRequestHandler):
    def do_GET(self):
        self.path = '/3ds' + self.path
        SimpleHTTPServer.SimpleHTTPRequestHandler.do_GET(self)
        if self.path[len(folder):] in files:
            files.remove(self.path[len(folder):])
            if len(files) == 0:
                def kill(server):
                    server.shutdown()
                thread.start_new_thread(kill, (httpd,))
        return

class MyTCPServer(SocketServer.TCPServer):
    def server_bind(self):
        import socket
        self.socket.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
        self.socket.bind(self.server_address)

server_address = ('', 8000)
httpd = MyTCPServer(server_address, MyHandler)
try:
    httpd.serve_forever()
except KeyboardInterrupt:
    pass
httpd.server_close()
