#!/usr/bin/env python
# Public domain, 2013 Simone Basso <bassosimone@gmail.com>.

""" Test for NeubotPollable """

import sys

sys.path.insert(0, "/usr/local/share/libneubot")

import libneubot

def periodic_callback(poller):
    """ The periodic callback """
    sys.stdout.write("Periodic callback\n")
    schedule_callback(poller)

PERIODIC_CALLBACK = libneubot.NEUBOT_HOOK_VO(periodic_callback)

def schedule_callback(poller):
    """ Schedule the periodic callback """
    libneubot.NeubotPoller_sched(poller, 1.0, PERIODIC_CALLBACK, poller)

def main():
    """ Main function """
    poller = libneubot.NeubotPoller_construct()
    schedule_callback(poller)
    libneubot.NeubotPoller_loop(poller)

if __name__ == "__main__":
    main()
