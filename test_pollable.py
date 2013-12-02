#!/usr/bin/env python
# Public domain, 2013 Simone Basso <bassosimone@gmail.com>.

""" Test for NeubotPollable """

import os
import sys

from libneubot import LIBNEUBOT
from libneubot import NEUBOT_POLLABLE_CALLBACK

def read_callback(pollable):
    """ Read callback """
    data = os.read(0, 1024)
    if not data:
        LIBNEUBOT.NeubotPollable_close(pollable)
        return
    sys.stdout.write(data)

def write_callback(pollable):
    """ Write callback """

def close_callback(pollable):
    """ Close callback """
    poller = LIBNEUBOT.NeubotPollable_poller(pollable)
    LIBNEUBOT.NeubotPoller_break_loop(poller)

READ_CALLBACK = NEUBOT_POLLABLE_CALLBACK(read_callback)
WRITE_CALLBACK = NEUBOT_POLLABLE_CALLBACK(write_callback)
CLOSE_CALLBACK = NEUBOT_POLLABLE_CALLBACK(close_callback)

def main():
    """ Main function """

    poller = LIBNEUBOT.NeubotPoller_construct()

    pollable = LIBNEUBOT.NeubotPollable_construct(poller, READ_CALLBACK,
      WRITE_CALLBACK, CLOSE_CALLBACK, None)
    LIBNEUBOT.NeubotPollable_attach(pollable, 0)
    LIBNEUBOT.NeubotPollable_set_readable(pollable)

    LIBNEUBOT.NeubotPoller_loop(poller)

if __name__ == "__main__":
    main()
