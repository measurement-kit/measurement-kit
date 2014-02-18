%module(directors="1") libneubot_python;
%feature("director");
%include "neubot.hh"
%{
#include "../neubot.hh"
%}
