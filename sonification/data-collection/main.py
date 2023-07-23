import threading

from pythonosc import udp_client
import subprocess
import os
import sys


def get_pids_of_program(name: str):
    return [int(pstr) for pstr in
            subprocess.run(["pidof", "-d,", name], stdout=subprocess.PIPE, text=True).stdout.split(',') if
            len(pstr) > 0]


program_input = sys.argv[1] if len(sys.argv) > 1 else input()
try:
    pids = [int(s) for s in program_input.split(',')]
except ValueError:
    pids = get_pids_of_program(program_input)


OSC_SERVER_ADDRESS = os.getenv("OSC_SERVER_ADDRESS") or "localhost"
OSC_SERVER_PORT = int(os.getenv("OSC_SERVER_PORT") or 9000)
INTERVAL_MS = int(os.getenv("INTERVAL_MS") or 10)

client = udp_client.SimpleUDPClient(OSC_SERVER_ADDRESS, OSC_SERVER_PORT)

from procfs_io import record_procfs_io
from perf import record_perf

recording_funcs = [
    record_procfs_io,
    record_perf
]

threads = [threading.Thread(target=fun, args=(client, pids, INTERVAL_MS)) for fun in recording_funcs]

for t in threads:
    t.start()
for t in threads:
    t.join()
