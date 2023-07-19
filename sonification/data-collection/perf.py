import math
import sys

from pythonosc import udp_client
import subprocess
import matplotlib
matplotlib.use("TkAgg")
import matplotlib.pyplot as plt


def plot_counter_data(ax, data, name):
    # plt.plot(timestamps, counts_normalized, marker='.')
    # plt.vlines(timestamps, ymin=0, ymax=counts_normalized, linewidth=10)
    # plt.step(timestamps, counts_normalized, where="pre")
    ax.fill_between(data['t'], data['density'], step="pre", alpha=1)
    # plt.ylim(bottom=0, top=1e8)
    ax.set_ylim(bottom=0)

    end_t = data['t'][-1]
    start_t = end_t - 10
    ax.set_xlim(left=start_t, right=end_t)
    ax.set_xlabel("time [s]")
    ax.set_ylabel("event density [1/s]")
    ax.set_title(name)




events = [
    "task-clock:u",
    "cycles:uD",
    "instructions:uD",
    "L1-dcache-load-misses:u",
    "L2_RQSTS.DEMAND_DATA_RD_MISS:u", #"MEM_LOAD_UOPS_RETIRED.L2_MISS:u",
    "LLC-load-misses:u", #"MEM_LOAD_UOPS_RETIRED.L3_MISS:u"
    "faults",
    "major-faults",
    "L1-icache-load-misses"
]

def record_perf(client: udp_client.SimpleUDPClient, pids: [int], interval_ms: int):
    if len(pids) == 0:
        return

    cmdline = f'perf stat --no-group --metric-no-group -e {(",".join(events))} -I {interval_ms} -x\\; --pid {",".join([str(p) for p in pids])}'
    # LANG=en_US is needed for having dots as the decimal separator
    with subprocess.Popen(cmdline, shell=True, stderr=subprocess.PIPE, pipesize=1048576, text=True, env={ "LANG": "en_US"}) as p:
        eventdata = [{ 't': [], 'density': [], 'delta_t': [] } for _ in events]

        granularity = -1
        j = 0
        k = 0

        if granularity > 0:
            fig, axes = plt.subplots(len(events), 1)

        try:
            while not p.stderr.closed:
                index = 0
                for lines in eventdata:
                    line = p.stderr.readline()
                    if len(line) == 0:
                        raise EOFError()
                    columns = line.strip().split(";")
                    timestamp = float(columns[0])
                    last_timestamp = lines['t'][-1] if len(lines['t']) > 0 else 0
                    lines['t'].append(timestamp)

                    count = float(columns[1]) if columns[1] != "<not counted>" else 0
                    unit = columns[2]
                    eventname = columns[3]
                    if not events[index].startswith(eventname):
                        raise ValueError()
                    multiplex_percent = float(columns[5])

                    delta_t = timestamp - last_timestamp
                    lines['delta_t'].append(delta_t)
                    count_density = count / delta_t if multiplex_percent != 0 else math.nan
                    lines['density'].append(count_density)
                    index += 1

                utilization = eventdata[0]['density'][-1] / 1000
                cycles = eventdata[1]['density'][-1]
                instructions = eventdata[2]['density'][-1]

                client.send_message("/task-clock", utilization)

                if not (math.isnan(cycles) or math.isnan(instructions)):
                    client.send_message("/IPC", [cycles, instructions])

                for name, eventindex in [("L1D", 3), ("L2", 4), ("L3", 5), ("L1I", 8)]:
                    misses_rel = 0 if instructions == 0 else eventdata[eventindex]['density'][-1]/instructions
                    client.send_message(f"/cache/{name}/misses", misses_rel)

                j += 1
                if j == granularity:
                    j = 0
                    for ax, lines, name in zip(axes.flat, eventdata, events):
                        ax.clear()
                        plot_counter_data(ax, lines, name)
                    plt.pause(0.001)

        except EOFError:
            pass

        # Ensure chuck gets quiet again
        for name, eventindex in [("L1D", 3), ("L2", 4), ("L3", 5), ("L1I", 8)]:
            client.send_message(f"/cache/{name}/misses", 0)
        client.send_message("/task-clock", 0)
        client.send_message("/IPC", [0, 0])
