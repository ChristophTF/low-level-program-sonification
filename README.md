# Low-Level Program Sonification

## Prerequisites

This prototype is developed for linux.
The following software is needed

- [chuck](https://chuck.cs.princeton.edu/)
- perf: Install using
```bash
apt-get install linux-tools-common linux-tools-generic linux-tools-`uname -r`
```
- python3
- CMake

## Getting started

1. Build the procfs reader:
```bash
$ cd ./sonification/data-collection/pid-children-transitive
$ mkdir cmake-build-release && cd cmake-build-release
$ cmake .. -DCMAKE_BUILD_TYPE=Release
$ make
```

2. Start the ChucK synthesizer:
```bash
$ cd ./sonification/sound-synthesis
$ ./chuck.sh
```

3. Start the data collection:
```bash
$ cd ./sonification/data-collection
$ python3 main.py
firefox
```
In the last line, you enter the program name or a comma separated list of process ids of the program that should be observed.