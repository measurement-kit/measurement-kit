#!/usr/bin/env python
# Public domain, 2013 Simone Basso <bassosimone@gmail.com>.

""" Test for NeubotPollable """

import ctypes
import os
import sys

sys.path.insert(0, "/usr/local/share/libneubot")

from libneubot import LIBNEUBOT
from libneubot import NEUBOT_SLOT_VO

def read_callback(ccontext):
    """ Read callback """
    poller, _ = ccontext
    data = os.read(0, 1024)
    if not data:
        LIBNEUBOT.NeubotPoller_break_loop(poller)
        return
    sys.stdout.write(data)

def write_callback(ccontext):
    """ Write callback """

def error_callback(ccontext):
    """ Error callback """
    poller, _ = ccontext
    sys.stderr.write("error!\n")
    LIBNEUBOT.NeubotPoller_break_loop(poller)

ERROR_CALLBACK = NEUBOT_SLOT_VO(error_callback)
READ_CALLBACK = NEUBOT_SLOT_VO(read_callback)
WRITE_CALLBACK = NEUBOT_SLOT_VO(write_callback)

def main():
    """ Main function """

    poller = LIBNEUBOT.NeubotPoller_construct()

    context = [poller]
    ccontext = ctypes.py_object(context)

    pollable = LIBNEUBOT.NeubotPollable_construct(poller, READ_CALLBACK,
      WRITE_CALLBACK, ERROR_CALLBACK, ccontext)

    context.append(pollable)

    LIBNEUBOT.NeubotPollable_attach(pollable, 0)
    LIBNEUBOT.NeubotPollable_set_timeout(pollable, 7.0)
    LIBNEUBOT.NeubotPollable_set_readable(pollable)

    LIBNEUBOT.NeubotPoller_loop(poller)

    LIBNEUBOT.NeubotPollable_close(pollable)

if __name__ == "__main__":
    main()
