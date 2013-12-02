#!/usr/bin/env python
# Public domain, 2013 Simone Basso <bassosimone@gmail.com>.

""" Test for NeubotEchoServer """

from libneubot import LIBNEUBOT

def main():
    """ Main function """
    poller = LIBNEUBOT.NeubotPoller_construct()
    LIBNEUBOT.NeubotEchoServer_construct(poller, 1, "::1", "12345")
    LIBNEUBOT.NeubotPoller_loop(poller)

if __name__ == "__main__":
    main()
