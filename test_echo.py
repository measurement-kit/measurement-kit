#!/usr/bin/env python
# Public domain, 2013 Simone Basso <bassosimone@gmail.com>.

""" Test for NeubotEchoServer """

import libneubot

def main():
    """ Main function """
    poller = libneubot.NeubotPoller_construct()
    libneubot.NeubotEchoServer_construct(poller, 0, "127.0.0.1", "12345")
    libneubot.NeubotPoller_loop(poller)

if __name__ == "__main__":
    main()
