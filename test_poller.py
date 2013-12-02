#!/usr/bin/env python
# Public domain, 2013 Simone Basso <bassosimone@gmail.com>.

""" Test for NeubotPoller """

# pylint: disable = W0212

import ctypes
import os
import sys

from libneubot import LIBNEUBOT
from libneubot import NEUBOT_POLLER_CALLBACK

def read_timeo(pyobject):
    """ Read timed out """
    poller = ctypes.cast(pyobject, ctypes.py_object).value
    sys.stderr.write("Timeout!\n")
    LIBNEUBOT.NeubotPoller_break_loop(poller)

def read_ok(pyobject):
    """ Can read from the socket """
    poller = ctypes.cast(pyobject, ctypes.py_object).value

    data = os.read(0, 1024)
    if not data:
        sys.stderr.write("EOF!\n")
        LIBNEUBOT.NeubotPoller_break_loop(poller)
        return

    sys.stdout.write(data)
    schedule_read(poller)

READ_OK = NEUBOT_POLLER_CALLBACK(read_ok)
READ_TIMEO = NEUBOT_POLLER_CALLBACK(read_timeo)

def schedule_read(poller):
    """ Schedule a read operation """
    nevp = LIBNEUBOT.NeubotPoller_defer_read(poller, 0, READ_OK, READ_TIMEO,
      poller, ctypes.c_double(10.0))
    if nevp == 0:
        os._exit(1)

def main():
    """ Main function """

    poller = LIBNEUBOT.NeubotPoller_construct()
    schedule_read(poller)
    LIBNEUBOT.NeubotPoller_loop(poller)

if __name__ == "__main__":
    main()
