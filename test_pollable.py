#!/usr/bin/env python
# Public domain, 2013 Simone Basso <bassosimone@gmail.com>.

""" Test for NeubotPollable """

import os
import sys

import libneubot

def read_callback(pollable):
    """ Read callback """
    data = os.read(0, 1024)
    if not data:
        libneubot.NeubotPollable_close(pollable)
        return
    sys.stdout.write(data)

def write_callback(pollable):
    """ Write callback """

def close_callback(pollable):
    """ Close callback """
    poller = libneubot.NeubotPollable_poller(pollable)
    libneubot.NeubotPoller_break_loop(poller)

READ_CALLBACK = libneubot.NEUBOT_POLLABLE_CALLBACK(read_callback)
WRITE_CALLBACK = libneubot.NEUBOT_POLLABLE_CALLBACK(write_callback)
CLOSE_CALLBACK = libneubot.NEUBOT_POLLABLE_CALLBACK(close_callback)

def main():
    """ Main function """

    poller = libneubot.NeubotPoller_construct()

    pollable = libneubot.NeubotPollable_construct(poller, READ_CALLBACK,
      WRITE_CALLBACK, CLOSE_CALLBACK, None)
    libneubot.NeubotPollable_attach(pollable, 0)
    libneubot.NeubotPollable_set_readable(pollable)

    libneubot.NeubotPoller_loop(poller)

if __name__ == "__main__":
    main()
