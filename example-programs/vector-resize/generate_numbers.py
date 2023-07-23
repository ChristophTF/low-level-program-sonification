#!/usr/bin/python3
import random
import sys

N = int(sys.argv[1])
print(N)
for i in range(N):
    print(random.randint(0, (1 << 64) - 1))