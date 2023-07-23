import subprocess
import sys

from pythonosc import udp_client

def record_procfs_io(client: udp_client.SimpleUDPClient, pids: [int], interval_ms: int):
    if len(pids) == 0:
        return

    pid_children_transitive_path = "pid-children-transitive/cmake-build-release/pid_children_transitive"
    proc_reader_path = "proc-reader/cmake-build-release/proc_reader"
    with subprocess.Popen(f'{pid_children_transitive_path} {(",".join([str(p) for p in pids]))} 100000000 | {proc_reader_path} {interval_ms * 1000 * 1000}', shell=True, stdout=subprocess.PIPE, pipesize=1048576, text=True) as p:
        while not p.stdout.closed:
            line = p.stdout.readline()
            if len(line) == 0:
                break
            stats = [int(token) for token in line.split(',')]
            syscalls_read = stats[2]
            syscalls_write = stats[3]
            bytes_read = stats[0]
            bytes_write = stats[1]
            storage_read = stats[4]
            storage_write = stats[5]

            if syscalls_read:
                client.send_message("/io/syscalls/read", [syscalls_read, bytes_read])
            if syscalls_write:
                client.send_message("/io/syscalls/write", [syscalls_write, bytes_write])
            if storage_read:
                client.send_message("/io/storage/read", [1, storage_read])
            if storage_write:
                client.send_message("/io/storage/write", [1, storage_write])

    print("procfs_io ended", file=sys.stderr)
