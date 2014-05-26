#!/usr/bin/env python
#Public domain, 2014 Alessandro Quaranta <alessandro.quaranta92@gmail.com>

import ctypes
import os
import sys

sys.path.insert(0, "/usr/local/share/libneubot")

from libneubot import Connection
from libneubot import Poller
from libneubot import Protocol

BUFF_LENGTH = 65535
BUFF_TYPE = ctypes.c_char * BUFF_LENGTH

def create_new_buffer():
    buff_in = bytearray(BUFF_LENGTH)
    return BUFF_TYPE.from_buffer(buff_in) 
    
def main():
    """ Main function """

    poller = Poller()
    proto = Protocol(poller)
    connection = Connection.attach(proto, 0)
    
    str_out = 'Hello, world!\n'

    sys.stdout.write('Connection::puts_readbuf() - Connection::read()\n')    
    llbuff_in = create_new_buffer() 
    connection.puts_readbuf(str_out)
    result = connection.read(llbuff_in, BUFF_LENGTH)
    sys.stdout.write('%d chars read: %s\n' % (result, llbuff_in.value))

    sys.stdout.write('Connection::write_readbuf() - Connection::read()\n')    
    llbuff_in = create_new_buffer() 
    connection.write_readbuf(str_out, len(str_out))
    result = connection.read(llbuff_in, BUFF_LENGTH)
    sys.stdout.write('%d chars read: %s\n' % (result, llbuff_in.value))
    
    sys.stdout.write('Connection::read() (expected empty string)\n')    
    llbuff_in = create_new_buffer() 
    result = connection.read(llbuff_in, BUFF_LENGTH)
    sys.stdout.write('%d chars read: %s\n\n' % (result, llbuff_in.value))
    
    sys.stdout.write('Connection::puts_readbuf() - Connection::readline()\n')    
    llbuff_in = create_new_buffer() 
    connection.puts_readbuf(str_out)
    result = connection.readline(llbuff_in, BUFF_LENGTH)
    sys.stdout.write('%d chars read: %s\n' % (result, llbuff_in.value))
    
    sys.stdout.write('Connection::write_rand_readbuf() - Connection::read()\n')
    llbuff_in = create_new_buffer() 
    connection.write_rand_readbuf(BUFF_LENGTH)
    result = connection.read(llbuff_in, BUFF_LENGTH)
    sys.stdout.write('%d characters read\n' %  result)

    connection.close()

if __name__ == "__main__":
    main()
