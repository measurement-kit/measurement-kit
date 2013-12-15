#!/usr/bin/env python
# Public domain, 2013 Simone Basso <bassosimone@gmail.com>.

""" Test for NeubotPollable """

import ctypes
import os
import sys

from libneubot1 import Pollable
from libneubot1 import Poller

class EchoPollable(Pollable):

    def __init__(self, poller):
        Pollable.__init__(self, poller)
        self.poller = poller

    def handle_read(self):
        data = os.read(0, 1024)
        if not data:
            self.close()
            return
        sys.stdout.write(data)

    def handle_close(self):
        self.poller.break_loop()

def main():
    poller = Poller()
    pollable = EchoPollable(poller)
    pollable.attach(0)
    pollable.set_readable()
    poller.loop()

if __name__ == "__main__":
    main()
