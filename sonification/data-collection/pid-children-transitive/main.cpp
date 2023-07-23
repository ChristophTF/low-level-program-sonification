#include <iostream>
#include <cstring>
#include <vector>
#include <dirent.h>
#include <unordered_set>
#include <stack>
#include <optional>

using namespace std;

static void search_transitive_children(unordered_set<pid_t>& result, unordered_set<pid_t>& start)
{
    char buf[1000];

    stack<pid_t> worklist;
    for(pid_t p : start)
        worklist.push(p);

    while(!worklist.empty())
    {
        pid_t p = worklist.top();
        worklist.pop();
        if (result.contains(p))
            continue;

        snprintf(buf, sizeof(buf), "/proc/%d/task", p);
        DIR* threads = opendir(buf);
        if (!threads)
        {
            if (errno == ESRCH || errno == ENOENT)
            {
                start.erase(p);
                continue;
            }
            perror("opendir");
            fprintf(stderr, "opendir path: %s\n", buf);
            exit(1);
        }

        if (result.insert(p).second)
        {
            while (dirent* entry = readdir(threads))
            {
                if (0 == strcmp(entry->d_name, ".") || 0 == strcmp(entry->d_name, ".."))
                    continue;

                snprintf(buf, sizeof(buf), "/proc/%d/task/%s/children", p, entry->d_name);
                FILE* children_f = fopen(buf, "r");
                if (!children_f)
                    break;

                for(;;)
                {
                    char* line = buf;
                    size_t linelen = sizeof(buf);
                    if (-1 == getline(&line, &linelen, children_f))
                        break;
                    pid_t child_p = atoi(line);
                    if (line != buf)
                        free(line);

                    worklist.push(child_p);
                }

                fclose(children_f);
            }
        }

        closedir(threads);
    }
}


static unordered_set<pid_t> parse_pids(char *pids_str)
{
    unordered_set<pid_t> pids;
    char *pid = strtok(pids_str, ",");
    while(pid)
    {
        pids.insert(atoi(pid));
        pid = strtok(nullptr, ",");
    }
    return pids;
}

static void print_pids(ostream& out, auto pids)
{
    size_t i = pids.size();
    for(pid_t p : pids)
    {
        out << p;
        if (--i == 0)
            out << endl;
        else
            out << ',';
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <pids> [<interval_ns>]\n", argv[0]);
        exit(1);
    }

    auto pids = parse_pids(argv[1]);
    unordered_set<pid_t> transitive_pids;

    search_transitive_children(transitive_pids, pids);
    if (transitive_pids.empty() && argc >= 3)
        exit(0);

    print_pids(cout, transitive_pids);

    if (argc < 3)
        exit(0);

    int64_t pause_ns = atoi(argv[2]);
    struct timespec pause { .tv_sec = pause_ns / 1000000000, .tv_nsec = pause_ns % 1000000000 };

    for(;;)
    {
        nanosleep(&pause, nullptr);

        unordered_set<pid_t> next_transitive_pids;
        search_transitive_children(next_transitive_pids, pids);
        if (next_transitive_pids.empty())
            exit(0);

        if (next_transitive_pids != transitive_pids)
        {
            transitive_pids = std::move(next_transitive_pids);
            print_pids(cout, transitive_pids);
            clog << "Found new set of pids: ";
            print_pids(clog, transitive_pids);
        }
    }
}
