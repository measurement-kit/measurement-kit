#!/usr/bin/env python
# Public domain, 2013 Simone Basso <bassosimone@gmail.com>.

""" Test for NeubotPollable """

import sys

sys.path.insert(0, "/usr/local/share/libneubot")

from libneubot import LIBNEUBOT
from libneubot import NEUBOT_HOOK_VOS

def resolve_callback(poller, addresses):
    """ The resolve callback """
    sys.stdout.write("addresses: %s\n" % addresses.split())
    LIBNEUBOT.NeubotPoller_break_loop(poller)

RESOLVE_CALLBACK = NEUBOT_HOOK_VOS(resolve_callback)

def main():
    """ Main function """
    poller = LIBNEUBOT.NeubotPoller_construct()
    LIBNEUBOT.NeubotPoller_resolve(poller, "PF_INET6", "www.youtube.com",
      RESOLVE_CALLBACK, poller)
    LIBNEUBOT.NeubotPoller_loop(poller)

if __name__ == "__main__":
    main()
