# Autogenerate API headers

This directory contains a script to automatically generate API headers from
a simple specification of Measurement Kit's FFI API messages.

## Content

- `README.md`: this file

- `autoapi`: message specification and autogen script

- `ffi_macros.h.j2`: template for `include/measurement_kit/ffi_macros.h`

- `nettest.hpp.j2`: template for `include/measurement_kit/nettest.hpp`

## Usage

Make sure Python3 is installed. Then run:

```
virtualenv venv
source venv/bin/activate
pip install jinja2
./script/autoapi/autoapi
deactivate
rm -rf venv
```
