#!/usr/bin/env python
# Public domain, 2013 Simone Basso <bassosimone@gmail.com>.

""" Test for NeubotPollable """

import ctypes
import os
import sys

import libneubot

def read_callback(ccontext):
    """ Read callback """
    _, pollable = ccontext
    data = os.read(0, 1024)
    if not data:
        libneubot.NeubotPollable_close(pollable)
        return
    sys.stdout.write(data)

def write_callback(ccontext):
    """ Write callback """

def close_callback(ccontext):
    """ Close callback """
    poller, _ = ccontext
    libneubot.NeubotPoller_break_loop(poller)

READ_CALLBACK = libneubot.NEUBOT_SLOT_VO(read_callback)
WRITE_CALLBACK = libneubot.NEUBOT_SLOT_VO(write_callback)
CLOSE_CALLBACK = libneubot.NEUBOT_SLOT_VO(close_callback)

def main():
    """ Main function """

    poller = libneubot.NeubotPoller_construct()

    context = [ poller ]
    ccontext = ctypes.py_object(context)

    pollable = libneubot.NeubotPollable_construct(READ_CALLBACK,
      WRITE_CALLBACK, CLOSE_CALLBACK, ccontext)

    context.append(pollable)

    libneubot.NeubotPollable_attach(pollable, poller, 0)
    libneubot.NeubotPollable_set_readable(pollable)

    libneubot.NeubotPoller_loop(poller)

if __name__ == "__main__":
    main()
