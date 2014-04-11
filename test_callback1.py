#!/usr/bin/env python
# Public domain, 2013 Simone Basso <bassosimone@gmail.com>.

""" Test for NeubotPoller_sched() """

import sys

from libneubot import Poller

def periodic_callback(poller):
    """ The periodic callback """
    sys.stdout.write("Periodic callback\n")
    poller.sched(1.0, periodic_callback, poller)

def main():
    """ Main function """
    poller = Poller()
    poller.sched(1.0, periodic_callback, poller)
    poller.loop()

if __name__ == "__main__":
    main()
