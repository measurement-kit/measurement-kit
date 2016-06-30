# Part of measurement-kit <https://measurement-kit.github.io/>.
# Measurement-kit is free software. See AUTHORS and LICENSE for more
# information on the copying conditions.

from __future__ import print_function

import _ctypes
import sys

from ctypes import (
    CDLL,
    CFUNCTYPE,
    c_char_p,
    c_int,
    c_uint32,
    c_ulong,
    c_void_p
)


MK_LOG_WARNING = 0
MK_LOG_INFO = 1
MK_LOG_DEBUG = 2


class Easy(object):

    done_fn_t = CFUNCTYPE(None, c_int)

    log_fn_t = CFUNCTYPE(None, c_uint32, c_char_p)

    apis = (
        (c_void_p, "new", (c_char_p, )),
        (None, "on_log", (c_void_p, log_fn_t)),
        (None, "set_verbosity", (c_void_p, c_uint32)),
        (None, "increase_verbosity", (c_void_p, )),
        (None, "set_input_filepath", (c_void_p, c_char_p)),
        (None, "set_output_filepath", (c_void_p, c_char_p)),
        (None, "set_options", (c_void_p, c_char_p, c_char_p)),
        (c_int, "run", (c_void_p, )),
        (None, "run_async", (c_void_p, done_fn_t)),
    )

    def __init__(self, library_path):
        self._handle = CDLL(library_path)
        if self._handle.mk_easy_abi_get() != 0:
            raise RuntimeError("unsupported ABI")
        for rvt, name, args in self.apis:
            func = getattr(self._handle, "mk_easy_" + name)
            func.argtypes = args
            func.restype = rvt
            self.__dict__[name] = func


class BaseTest(object):

    def __init__(self, easy, name):
        self._easy = easy
        self._handle = self._easy.new(name)
        self._log_callback = None
        self._done_callback = None
        assert(self._handle)

    def on_log(self, callback):
        self._log_callback = self._easy.log_fn_t(callback)
        self._easy.on_log(self._handle, self._log_callback)
        return self

    def set_verbosity(self, value):
        self._easy.set_verbosity(self._handle, value)
        return self

    def increase_verbosity(self):
        self._easy.increase_verbosity(self._handle)
        return self

    def set_input_filepath(self, value):
        self._easy.set_input_filepath(self._handle, value)
        return self

    def set_output_filepath(self, value):
        self._easy.set_output_filepath(self._handle, value)
        return self

    def set_options(self, key, value):
        self._easy.set_options(self._handle, key, value)
        return self

    def run(self):
        self._easy.run(self._handle)

    def run_async(self, callback):
        def done(result):
            _ctypes.Py_DECREF(self)
            callback(result)
        self._done_callback = self._easy.done_fn_t(done)
        _ctypes.Py_INCREF(self)
        self._easy.run_async(self._handle, self._done_callback)

    def __del__(self):
        if self._handle:
            self._easy.delete(self._handle)
            self._handle = None


class OoniHttpInvalidRequestLine(BaseTest):
    def __init__(self, easy):
        super(self.__class__, self).__init__(easy,
              b"ooni/http_invalid_request_line")


class OoniTcpConnect(BaseTest):
    def __init__(self, easy):
        super(self.__class__, self).__init__(easy,
              b"ooni/tcp_connect")


def main():
    import os
    import time

    easy = Easy(".libs/libmeasurement_kit.so")
    count = [0]

    def log_func(verbosity, message):
        print("<{0}> {1}".format(verbosity, message))

    def done_func(result):
        count[0] -= 1

    OoniHttpInvalidRequestLine(easy)                                     \
        .on_log(log_func)                                                \
        .set_verbosity(MK_LOG_INFO)                                      \
        .increase_verbosity()                                            \
        .set_options(b"geoip_country_path", b"test/fixtures/GeoIP.dat")  \
        .set_options(b"geoip_asn_path", b"test/fixtures/GeoIPASNum.dat") \
        .set_options(b"backend", b"http://213.138.109.232/")             \
        .run_async(done_func)

    count[0] += 1

    OoniTcpConnect(easy)                                                 \
        .on_log(log_func)                                                \
        .set_verbosity(MK_LOG_INFO)                                      \
        .increase_verbosity()                                            \
        .set_input_filepath(b"test/fixtures/hosts.txt")                  \
        .set_options(b"geoip_country_path", b"test/fixtures/GeoIP.dat")  \
        .set_options(b"geoip_asn_path", b"test/fixtures/GeoIPASNum.dat") \
        .set_options(b"backend", b"http://213.138.109.232/")             \
        .run_async(done_func)

    count[0] += 1

    while count[0] > 0:
        time.sleep(1)


if __name__ == "__main__":
    main()
