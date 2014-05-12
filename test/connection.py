#!/usr/bin/env python
# Public domain, 2014 Simone Basso <bassosimone@gmail.com>.

import ctypes
import os
import sys

sys.path.insert(0, "/usr/local/share/libneubot")

from libneubot import LIBNEUBOT
from libneubot import NEUBOT_SLOT_VO

# See https://mail.python.org/pipermail/python-list/2010-January/563868.html
BUFF_LENGTH = 65535
BUFF_TYPE = ctypes.c_char * BUFF_LENGTH

def _terminate(ccontext):
    connection = ccontext[0]
    proto = LIBNEUBOT.NeubotConnection_get_protocol(connection)
    if not proto:
        sys.exit("connection - cannot get protocol")
    poller = LIBNEUBOT.NeubotProtocol_get_poller(proto)
    if not poller:
        sys.exit("connection - cannot get poller")
    LIBNEUBOT.NeubotPoller_break_loop(poller)

def connect_callback(ccontext):
    sys.exit("connection - unexpected connect event")

def data_callback(ccontext):
    connection = ccontext[0]
    buff = bytearray(BUFF_LENGTH)
    llbuff = BUFF_TYPE.from_buffer(buff)
    while True:
        result = LIBNEUBOT.NeubotConnection_read(connection,
          llbuff, BUFF_LENGTH)
        if result < 0:
            sys.exit("connection - read failed")
        if result == 0:
            sys.stderr.write("connection - exhausted input buffer\n")
            break
        result = LIBNEUBOT.NeubotConnection_write(connection,
          llbuff, result)
        if result != 0:
            sys.exit("connection - write failed")

def ssl_callback(ccontext):
    sys.stderr.write("connection - SSL\n")

def flush_callback(ccontext):
    sys.stderr.write("connection - flushed\n")

def eof_callback(ccontext):
    sys.stderr.write("connection - eof\n")
    _terminate(ccontext)

def error_callback(ccontext):
    sys.stderr.write("connection - error\n")
    _terminate(ccontext)

CONNECT_CALLBACK = NEUBOT_SLOT_VO(connect_callback)
SSL_CALLBACK = NEUBOT_SLOT_VO(ssl_callback)
DATA_CALLBACK = NEUBOT_SLOT_VO(data_callback)
FLUSH_CALLBACK = NEUBOT_SLOT_VO(flush_callback)
EOF_CALLBACK = NEUBOT_SLOT_VO(eof_callback)
ERROR_CALLBACK = NEUBOT_SLOT_VO(error_callback)

def main():
    """ Main function """

    poller = LIBNEUBOT.NeubotPoller_construct()
    if not poller:
        sys.exit("connection - cannot construct the poller")

    context = []
    ccontext = ctypes.py_object(context)

    proto = LIBNEUBOT.NeubotProtocol_construct(poller, CONNECT_CALLBACK,
      SSL_CALLBACK, DATA_CALLBACK, FLUSH_CALLBACK, EOF_CALLBACK,
      ERROR_CALLBACK, ccontext)
    if not proto:
        sys.exit("connection - cannot construct the protocol")

    connection = LIBNEUBOT.NeubotConnection_attach(proto, 0)
    if not connection:
        sys.exit("connection - cannot attach the connection")

    result = LIBNEUBOT.NeubotConnection_set_timeout(connection, 7.0)
    if result != 0:
        sys.exit("connection - cannot set the timeout")

    context.append(connection)

    LIBNEUBOT.NeubotPoller_loop(poller)

if __name__ == "__main__":
    main()
