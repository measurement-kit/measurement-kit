#!/usr/bin/env python
# Public domain, 2013 Simone Basso <bassosimone@gmail.com>.

""" Test for IghtPollable """

import ctypes
import os
import sys

sys.path.insert(0, "/usr/local/share/libight")

from _libight import Pollable
from _libight import Poller

class EchoPollable(Pollable):

    def __init__(self, poller):
        Pollable.__init__(self, poller)
        self.poller = poller

    def handle_read(self):
        data = os.read(0, 1024)
        if not data:
            self.poller.break_loop()
            return
        sys.stdout.write(data)

    def handle_error(self):
        sys.stderr.write("error!\n")
        self.poller.break_loop()

def main():
    poller = Poller()
    pollable = EchoPollable(poller)
    pollable.attach(0)
    pollable.set_timeout(7.0)
    pollable.set_readable()
    poller.loop()
    pollable.close()

if __name__ == "__main__":
    main()
