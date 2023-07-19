import threading

from pythonosc import udp_client
import subprocess
import os



def run_program():
    input = open("/home/christoph/Uni/HPI/WiSe2223/ACP2/4-tree/ipomc10/large.in", "r")
    output = open("/dev/null", "w")
    p2 = subprocess.Popen("/home/christoph/Uni/HPI/WiSe2223/ACP2/4-tree/ipomc10/cmake-build-debug/ipomc10", shell=False,
                          stdin=input, stdout=output)
    pid = p2.pid
    def wait_for_exit():
        p2.wait()

    threading.Thread(target=wait_for_exit).start()
    return pid

def get_pids_of_program(name: str):
    return [int(pstr) for pstr in subprocess.run(["pidof", "-d,", name], stdout=subprocess.PIPE, text=True).stdout.split(',') if len(pstr) > 0]

#pids = [run_program()]
#pids = [int(pstr) for pstr in subprocess.run(["pidof", "-d,", "firefox"], stdout=subprocess.PIPE, text=True).stdout.split(',') if len(pstr) > 0]
#pids += [13640]
pids = get_pids_of_program("spotify")

OSC_SERVER_ADDRESS = os.getenv("OSC_SERVER_ADDRESS") or "localhost"
OSC_SERVER_PORT = int(os.getenv("OSC_SERVER_PORT") or 9000)
INTERVAL_MS = int(os.getenv("INTERVAL_MS") or 10)

client = udp_client.SimpleUDPClient(OSC_SERVER_ADDRESS, OSC_SERVER_PORT)

from procfs_io import record_procfs_io
from perf import record_perf

threads = [threading.Thread(target=fun, args=(client,pids,INTERVAL_MS)) for fun in [record_procfs_io, record_perf]]

for t in threads:
    t.start()
for t in threads:
    t.join()

