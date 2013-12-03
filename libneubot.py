#
# LibNeubot interface - Public domain.
#

import ctypes
import sys

if sys.platform == "darwin":
    LIBNEUBOT_NAME = "/usr/local/lib/libneubot.dylib.0"
else:
    LIBNEUBOT_NAME = "/usr/local/lib/libneubot.so.0"

LIBNEUBOT = ctypes.CDLL(LIBNEUBOT_NAME)

# Classes:

# struct NeubotEchoServer
# struct NeubotEvent
# struct NeubotPollable
# struct NeubotPoller

# Callbacks:

NEUBOT_POLLER_CALLBACK = ctypes.CFUNCTYPE(None, ctypes.c_void_p)
NEUBOT_POLLABLE_CALLBACK = ctypes.CFUNCTYPE(None, ctypes.c_void_p)

# NeubotEchoServer API:

LIBNEUBOT.NeubotEchoServer_construct.restype = ctypes.c_void_p
LIBNEUBOT.NeubotEchoServer_construct.argtypes = (ctypes.c_void_p, ctypes.c_int, ctypes.c_char_p, ctypes.c_char_p, )

# NeubotEvent API:

LIBNEUBOT.NeubotEvent_cancel.argtypes = (ctypes.c_void_p, )

# NeubotPollable API:

LIBNEUBOT.NeubotPollable_construct.restype = ctypes.c_void_p
LIBNEUBOT.NeubotPollable_construct.argtypes = (ctypes.c_void_p, NEUBOT_POLLABLE_CALLBACK, NEUBOT_POLLABLE_CALLBACK, NEUBOT_POLLABLE_CALLBACK, ctypes.c_void_p, )

LIBNEUBOT.NeubotPollable_opaque.restype = ctypes.c_void_p
LIBNEUBOT.NeubotPollable_opaque.argtypes = (ctypes.c_void_p, )

LIBNEUBOT.NeubotPollable_poller.restype = ctypes.c_void_p
LIBNEUBOT.NeubotPollable_poller.argtypes = (ctypes.c_void_p, )

LIBNEUBOT.NeubotPollable_attach.restype = ctypes.c_int
LIBNEUBOT.NeubotPollable_attach.argtypes = (ctypes.c_void_p, ctypes.c_longlong, )

LIBNEUBOT.NeubotPollable_detach.argtypes = (ctypes.c_void_p, )

LIBNEUBOT.NeubotPollable_fileno.restype = ctypes.c_longlong
LIBNEUBOT.NeubotPollable_fileno.argtypes = (ctypes.c_void_p, )

LIBNEUBOT.NeubotPollable_set_readable.restype = ctypes.c_int
LIBNEUBOT.NeubotPollable_set_readable.argtypes = (ctypes.c_void_p, )

LIBNEUBOT.NeubotPollable_unset_readable.restype = ctypes.c_int
LIBNEUBOT.NeubotPollable_unset_readable.argtypes = (ctypes.c_void_p, )

LIBNEUBOT.NeubotPollable_set_writable.restype = ctypes.c_int
LIBNEUBOT.NeubotPollable_set_writable.argtypes = (ctypes.c_void_p, )

LIBNEUBOT.NeubotPollable_unset_writable.restype = ctypes.c_int
LIBNEUBOT.NeubotPollable_unset_writable.argtypes = (ctypes.c_void_p, )

LIBNEUBOT.NeubotPollable_set_timeout.argtypes = (ctypes.c_void_p, ctypes.c_double, )

LIBNEUBOT.NeubotPollable_clear_timeout.argtypes = (ctypes.c_void_p, )

LIBNEUBOT.NeubotPollable_close.argtypes = (ctypes.c_void_p, )

# NeubotPoller API:

LIBNEUBOT.NeubotPoller_construct.restype = ctypes.c_void_p
LIBNEUBOT.NeubotPoller_construct.argtypes = ()

LIBNEUBOT.NeubotPoller_sched.restype = ctypes.c_int
LIBNEUBOT.NeubotPoller_sched.argtypes = (ctypes.c_void_p, ctypes.c_double, NEUBOT_POLLER_CALLBACK, ctypes.py_object, )

LIBNEUBOT.NeubotPoller_defer_read.restype = ctypes.c_void_p
LIBNEUBOT.NeubotPoller_defer_read.argtypes = (ctypes.c_void_p, ctypes.c_longlong, NEUBOT_POLLER_CALLBACK, NEUBOT_POLLER_CALLBACK, ctypes.py_object, ctypes.c_double, )

LIBNEUBOT.NeubotPoller_defer_write.restype = ctypes.c_void_p
LIBNEUBOT.NeubotPoller_defer_write.argtypes = (ctypes.c_void_p, ctypes.c_longlong, NEUBOT_POLLER_CALLBACK, NEUBOT_POLLER_CALLBACK, ctypes.py_object, ctypes.c_double, )

LIBNEUBOT.NeubotPoller_loop.argtypes = (ctypes.c_void_p, )

LIBNEUBOT.NeubotPoller_break_loop.argtypes = (ctypes.c_void_p, )

