#!/usr/bin/env python
# Public domain, 2014 Simone Basso <bassosimone@gmail.com>.

import ctypes
import os
import sys

sys.path.insert(0, "/usr/local/share/libneubot")

from libneubot import Connection
from libneubot import Poller
from libneubot import Protocol

# See https://mail.python.org/pipermail/python-list/2010-January/563868.html
BUFF_LENGTH = 65535
BUFF_TYPE = ctypes.c_char * BUFF_LENGTH

def _terminate(self):
    proto = self.connection.get_protocol()
    if not proto:
        sys.exit("connection - cannot get protocol")
    poller = self.get_poller()
    if not poller:
        sys.exit("connection - cannot get poller")
    poller.break_loop()

class MyProtocol(Protocol):

    def handle_connect(self):
        sys.exit("connection - unexpected connect event")

    def handle_data(self):
        buff = bytearray(BUFF_LENGTH)
        llbuff = BUFF_TYPE.from_buffer(buff)
        while True:
            result = self.connection.read(llbuff, BUFF_LENGTH)
            if result < 0:
                sys.exit("connection - read failed")
            if result == 0:
                sys.stderr.write("connection - exhausted input buffer\n")
                break
            result = self.connection.write(llbuff, result)
            if result != 0:
                sys.exit("connection - write failed")

    def handle_ssl(self):
        sys.stderr.write("connection - SSL\n")

    def handle_flush(self):
        sys.stderr.write("connection - flushed\n")

    def handle_eof(self):
        sys.stderr.write("connection - eof\n")
        _terminate(self)

    def handle_error(self):
        sys.stderr.write("connection - error\n")
        _terminate(self)


def main():
    """ Main function """

    poller = Poller() 
    if not poller:
        sys.exit("connection - cannot construct the poller")

    proto = MyProtocol(poller)
    if not proto:
        sys.exit("connection - cannot construct the protocol")

    proto.connection = Connection.attach(proto, 0)
    if not proto.connection:
        sys.exit("connection - cannot attach the connection")

    result = proto.connection.set_timeout(7.0)
    if result != 0:
        sys.exit("connection - cannot set the timeout")

    if proto.connection.enable_read() != 0:
        sys.exit("connection - cannot enable read")
    
    poller.loop() 

if __name__ == "__main__":
    main()
