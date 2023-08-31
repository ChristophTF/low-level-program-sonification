#!/usr/bin/python3

import random
import sys

MAX_NUMBER = 2 ** 64 - 1

t = int(sys.argv[1])

print(t)

for _ in range(t):
    print(random.randint(1, MAX_NUMBER))