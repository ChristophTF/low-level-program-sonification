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


TASK_CLOCK = "task-clock:uD"
CYCLES = "cycles:u"
INSTRUCTIONS = "instructions:u"
L1I_CACHE_MISSES = "L1-icache-load-misses:u"

L1D_CACHE_MISSES = "L1-dcache-load-misses:u"
L2_CACHE_MISSES = "L2_RQSTS.DEMAND_DATA_RD_MISS:u" #"MEM_LOAD_UOPS_RETIRED.L2_MISS:u"
L3_CACHE_MISSES = "LLC-load-misses:u" #"MEM_LOAD_UOPS_RETIRED.L3_MISS:u"

FAULTS = "faults:D"
MAJOR_FAULTS = "major-faults:D"

BRANCHES = "branches:u"
BRANCH_MISSES = "branch-misses:u"


events = [
    TASK_CLOCK,
    [INSTRUCTIONS, CYCLES],
    L1I_CACHE_MISSES,
    L1D_CACHE_MISSES,
    L2_CACHE_MISSES,
    L3_CACHE_MISSES,
    FAULTS,
    MAJOR_FAULTS,
    [BRANCHES, BRANCH_MISSES]
]

events_flat = [event for group in events for event in ([group] if type(group) is str else group)]

def record_perf(client: udp_client.SimpleUDPClient, pids: [int], interval_ms: int):
    if len(pids) == 0:
        return

    P = subprocess.run(["pid-children-transitive/cmake-build-release/pid_children_transitive", ",".join([str(p) for p in pids])], capture_output=True, text=True)
    child_pids = P.stdout.strip('\n')

    events_in_groups = [group if type(group) is str else f'{{{",".join(group)}}}' for group in events]
    events_str = ",".join(events_in_groups)

    cmdline = f'perf stat -o /dev/stdout -e {events_str} -I {interval_ms} -x\\; --pid {child_pids}'
    # LANG=en_US is needed for having dots as the decimal separator
    with subprocess.Popen(cmdline, shell=True, stdout=subprocess.PIPE, pipesize=1048576, text=True, env={ "LANG": "en_US"}) as p:
        eventdata = { name: { 't': [], 'density': [], 'delta_t': [] } for name in events_flat}

        granularity = -1
        j = 0
        k = 0

        if granularity > 0:
            fig, axes = plt.subplots(len(events_flat), 1)

        firstline = p.stdout.readline()
        secondline = p.stdout.readline()

        try:
            while not p.stdout.closed:
                for event_name, lines in eventdata.items():
                    line = p.stdout.readline()
                    if len(line) == 0:
                        raise EOFError()
                    columns = line.strip().split(";")
                    timestamp = float(columns[0])
                    last_timestamp = lines['t'][-1] if len(lines['t']) > 0 else 0
                    lines['t'].append(timestamp)

                    count = float(columns[1]) if columns[1] != "<not counted>" else 0
                    unit = columns[2]
                    if not event_name.startswith(columns[3]):
                        raise ValueError()
                    multiplex_percent = float(columns[5])

                    delta_t = timestamp - last_timestamp
                    lines['delta_t'].append(delta_t)
                    count_density = count / delta_t if multiplex_percent != 0 else math.nan
                    lines['density'].append(count_density)

                utilization = eventdata[TASK_CLOCK]['density'][-1] / 1000
                cycles = eventdata[CYCLES]['density'][-1]
                instructions = eventdata[INSTRUCTIONS]['density'][-1]

                client.send_message("/task-clock", utilization)

                if not (math.isnan(cycles) or math.isnan(instructions)):
                    client.send_message("/IPC", [cycles, instructions])

                for osc_name, event_name in [("L1I", L1I_CACHE_MISSES), ("L1D", L1D_CACHE_MISSES), ("L2", L2_CACHE_MISSES), ("L3", L3_CACHE_MISSES)]:
                    misses_rel = math.nan if instructions == 0 else eventdata[event_name]['density'][-1]/instructions
                    if not math.isnan(misses_rel):
                        misses_rel = min(misses_rel, 0.3)  # Clean data from sampling-related outliers
                        client.send_message(f"/cache/{osc_name}/misses", misses_rel)

                branches = eventdata[BRANCHES]['density'][-1]
                branch_misses = eventdata[BRANCH_MISSES]['density'][-1]

                if not (math.isnan(branches) or math.isnan(branch_misses)):
                    client.send_message("/branches", [branches, branch_misses])

                j += 1
                if j == granularity:
                    j = 0
                    for ax, (event_name, lines) in zip(axes.flat, eventdata.items()):
                        ax.clear()
                        plot_counter_data(ax, lines, event_name)
                    plt.pause(0.001)

        except EOFError:
            pass

        # Ensure chuck gets quiet again
        for osc_name in ["L1I", "L1D", "L2", "L3"]:
            client.send_message(f"/cache/{osc_name}/misses", 0)
        client.send_message("/task-clock", 0)
        client.send_message("/IPC", [0, 0])
