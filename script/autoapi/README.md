# Autogenerate API headers

This directory contains a script to automatically generate API headers from
a simple specification of Measurement Kit's FFI API messages.

## Content

- `README.md`: this file

- `autoapi`: message specification and autogen script

- `engine.cpp.j2`: template for `src/libmeasurement_kit/engine.cpp`

- `nettest.hpp.j2`: template for `include/measurement_kit/nettest.hpp`

## Usage

Make sure Python is installed (both Python 2 and Python 3 are fine). Then run:

```
virtualenv venv
source venv/bin/activate
pip install jinja2
./script/autoapi/autoapi
deactivate
rm -rf venv
```
