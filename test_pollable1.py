#!/usr/bin/env python
# Public domain, 2013 Simone Basso <bassosimone@gmail.com>.

""" Test for NeubotPollable """

import ctypes
import os
import sys

from libneubot1 import Pollable
from libneubot1 import Poller

class EchoPollable(Pollable):

    def __init__(self):
        Pollable.__init__(self)
        self.poller = None

    def attach(self, poller, filenum):
        self.poller = poller
        Pollable.attach(self, poller, filenum)

    def handle_read(self):
        data = os.read(0, 1024)
        if not data:
            self.poller.break_loop()
            return
        sys.stdout.write(data)

    def handle_error(self):
        self.close()

def main():
    poller = Poller()
    pollable = EchoPollable()
    pollable.attach(poller, 0)
    pollable.set_readable()
    poller.loop()

if __name__ == "__main__":
    main()
