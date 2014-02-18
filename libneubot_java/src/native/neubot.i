%module(directors="1") LibNeubot;
%feature("director");
%include "neubot.hh"
%{
#include "../neubot.hh"
%}
