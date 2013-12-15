#!/usr/bin/env python
# Public domain, 2013 Simone Basso <bassosimone@gmail.com>.

""" Test for NeubotPoller """

# pylint: disable = W0212

import ctypes
import os
import sys

import libneubot

def read_timeo(poller):
    """ Read timed out """
    sys.stderr.write("Timeout!\n")
    libneubot.NeubotPoller_break_loop(poller)

def read_ok(poller):
    """ Can read from the socket """

    data = os.read(0, 1024)
    if not data:
        sys.stderr.write("EOF!\n")
        libneubot.NeubotPoller_break_loop(poller)
        return

    sys.stdout.write(data)
    schedule_read(poller)

READ_OK = libneubot.NEUBOT_POLLER_CALLBACK(read_ok)
READ_TIMEO = libneubot.NEUBOT_POLLER_CALLBACK(read_timeo)

def schedule_read(poller):
    """ Schedule a read operation """
    libneubot.NeubotPoller_defer_read(poller, 0, READ_OK, READ_TIMEO,
      poller, ctypes.c_double(10.0))

def main():
    """ Main function """

    poller = libneubot.NeubotPoller_construct()
    schedule_read(poller)
    libneubot.NeubotPoller_loop(poller)

if __name__ == "__main__":
    main()
