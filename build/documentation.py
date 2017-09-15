#!/usr/bin/env python

import collections
import pprint
import subprocess
import sys

def gather_code(iterable):
    code = []
    for line in iterable:
        if (not line.strip().startswith("//") and
            not line.strip().startswith("#include")):
            code.append(line)
    proc = subprocess.Popen(["clang-format"], stdin=subprocess.PIPE,
                            stdout=subprocess.PIPE)
    return proc.communicate("".join(code))[0]

def write_synopsis(source, dest):
    dest.write("# SYNOPSIS\n\n```C++\n")
    dest.write(gather_code(source))
    dest.write("```\n\n")

def gather_doxygen(iterable):
    paragraphs = []
    current = []
    for line in iterable:
        eop = False
        line = line.lstrip()
        if line.startswith("///"):
            line = line.replace("///", "")
            line = line.strip()
            line = line.replace(r"\brief ", "")
            line = line.replace(r"\param ", "Parameter ")
            line = line.replace(r"\throw ", "Throws ")
            line = line.replace(r"\return ", "Returns ")
            line = line.replace(r"\since ", "Available since measurement-kit ")
            line = line.replace(r"\bug ", "_BUG_: ")
            line = line.replace(r"\note ", "_Note_: ")
            line = line.replace(r"\p ", "")
            line = line.replace(r"\see ", "See ")
            line = line.replace(r"\file ", "Documentation for ")
            current.append(line)
            if not line:
                eop = True
        else:
            eop = True
        if eop:
            paragraphs.append(" ".join(current))
            current = []
    if current:
        paragraphs.append(" ".join(current))
    paragraphs = [elem for elem in paragraphs if bool(elem)]
    return "\n\n".join(paragraphs)

def write_description(source, dest):
    dest.write("# DESCRIPTION\n\n")
    dest.write(gather_doxygen(source))
    dest.write("\n\n")

def build_documentation_for_file(include_path):
    normalized = include_path.replace("include/", "")
    output_path = (include_path.replace("include/measurement_kit", "doc/api")
            .replace(".hpp", ".md").replace(".h", ".md"))
    outfp = open(output_path, "wb")
    outfp.write("# NAME\n\n")
    outfp.write("`" + normalized + "`")
    outfp.write("\n\n")
    outfp.write("# LIBRARY\n\n")
    outfp.write(
            "measurement-kit (`libmeasurement_kit`, `-lmeasurement_kit`)\n\n")
    lines = list(open(include_path, "rb"))
    write_synopsis(lines, outfp)
    write_description(lines, outfp)

def main():
    if len(sys.argv) < 2:
        sys.exit("usage: ./build/documentation.py <file>")
    for arg in sys.argv[1:]:
        build_documentation_for_file(arg)

if __name__ == "__main__":
    main()
