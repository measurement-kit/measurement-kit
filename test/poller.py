#!/usr/bin/env python
# Public domain, 2013 Simone Basso <bassosimone@gmail.com>.

""" Test for NeubotPoller """

# pylint: disable = W0212

import ctypes
import os
import sys

sys.path.insert(0, "/usr/local/share/libneubot")

from libneubot import LIBNEUBOT
from libneubot import NEUBOT_HOOK_VO

def read_timeo(poller):
    """ Read timed out """
    sys.stderr.write("Timeout!\n")
    LIBNEUBOT.NeubotPoller_break_loop(poller)

def read_ok(poller):
    """ Can read from the socket """

    data = os.read(0, 1024)
    if not data:
        sys.stderr.write("EOF!\n")
        LIBNEUBOT.NeubotPoller_break_loop(poller)
        return

    sys.stdout.write(data)
    schedule_read(poller)

READ_OK = NEUBOT_HOOK_VO(read_ok)
READ_TIMEO = NEUBOT_HOOK_VO(read_timeo)

def schedule_read(poller):
    """ Schedule a read operation """
    LIBNEUBOT.NeubotPoller_defer_read(poller, 0, READ_OK, READ_TIMEO,
      poller, ctypes.c_double(10.0))

def main():
    """ Main function """

    poller = LIBNEUBOT.NeubotPoller_construct()
    schedule_read(poller)
    LIBNEUBOT.NeubotPoller_loop(poller)

if __name__ == "__main__":
    main()
