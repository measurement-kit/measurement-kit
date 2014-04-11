#!/usr/bin/env python
# Public domain, 2013 Simone Basso <bassosimone@gmail.com>.

""" Test for NeubotPoller """

# pylint: disable = C0111

import os
import sys

from libneubot import Poller

def read_timeo(poller):
    sys.stderr.write("Timeout!\n")
    poller.break_loop()

def read_ok(poller):
    data = os.read(0, 1024)
    if not data:
        sys.stderr.write("EOF!\n")
        poller.break_loop()
        return
    sys.stdout.write(data)
    poller.defer_read(0, read_ok, read_timeo, poller, 10.0)

def main():
    poller = Poller()
    poller.defer_read(0, read_ok, read_timeo, poller, 10.0)
    poller.loop()

if __name__ == "__main__":
    main()
