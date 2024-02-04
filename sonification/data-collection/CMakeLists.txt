cmake_minimum_required(VERSION 3.8)

project(data-collection LANGUAGES C CXX)

add_subdirectory("pid-children-transitive")
add_subdirectory("proc-reader")

file(COPY main.py perf.py procfs_io.py
     DESTINATION .)