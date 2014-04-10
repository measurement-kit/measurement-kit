#!/usr/bin/env python
# Public domain, 2013 Simone Basso <bassosimone@gmail.com>.

""" Test for NeubotPoller.resolve() """

import sys

from libneubot1 import Poller

def resolve_callback(poller, addresses):
    """ The resolve callback """
    sys.stdout.write("addresses: %s\n" % addresses.split())
    poller.break_loop()

def main():
    """ Main function """
    poller = Poller()
    poller.resolve("PF_INET", "www.youtube.com", resolve_callback, poller)
    poller.loop()

if __name__ == "__main__":
    main()
