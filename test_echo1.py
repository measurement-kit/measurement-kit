#!/usr/bin/env python
# Public domain, 2013 Simone Basso <bassosimone@gmail.com>.

""" Test for NeubotEchoServer """

from libneubot1 import EchoServer
from libneubot1 import Poller

def main():
    """ Main function """
    poller = Poller()
    EchoServer(poller, 0, "127.0.0.1", "12345")
    poller.loop()

if __name__ == "__main__":
    main()
