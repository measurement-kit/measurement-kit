#!/usr/bin/env python
# Public domain, 2013 Simone Basso <bassosimone@gmail.com>.

""" Test for NeubotPollable """

import ctypes
import sys

import libneubot

def resolve_callback(poller, address):
    """ The periodic callback """
    sys.stdout.write("address: %s\n" % address)
    libneubot.NeubotPoller_break_loop(poller)

RESOLVE_CALLBACK = libneubot.NEUBOT_HOOK_VOS(resolve_callback)

def main():
    """ Main function """
    poller = libneubot.NeubotPoller_construct()
    libneubot.NeubotPoller_resolve(poller, 0, "www.youtube.com",
      RESOLVE_CALLBACK, poller)
    libneubot.NeubotPoller_loop(poller)

if __name__ == "__main__":
    main()
