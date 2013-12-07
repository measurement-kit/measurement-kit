#!/usr/bin/env python
# Public domain, 2013 Simone Basso <bassosimone@gmail.com>.

""" Test for NeubotPollable """

import ctypes
import sys

from libneubot import LIBNEUBOT
from libneubot import NEUBOT_POLLER_RESOLVE_CALLBACK

def resolve_callback(opaque, address):
    """ The periodic callback """
    sys.stdout.write("address: %s\n" % address)
    poller = ctypes.cast(opaque, ctypes.py_object).value
    LIBNEUBOT.NeubotPoller_break_loop(poller)

RESOLVE_CALLBACK = NEUBOT_POLLER_RESOLVE_CALLBACK(resolve_callback)

def main():
    """ Main function """
    poller = LIBNEUBOT.NeubotPoller_construct()
    LIBNEUBOT.NeubotPoller_resolve(poller, 0, "www.youtube.com",
      RESOLVE_CALLBACK, poller)
    LIBNEUBOT.NeubotPoller_loop(poller)

if __name__ == "__main__":
    main()
