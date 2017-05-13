#!/usr/bin/env python
import json, sys
for line in open(sys.argv[1], "rb"):
    json.dump(json.loads(line.strip()), sys.stdout, indent=4, sort_keys=True)
    sys.stdout.write("\n\n")
