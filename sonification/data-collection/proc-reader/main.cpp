#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cstring>
#include <vector>
#include <dirent.h>
#include <unordered_set>
#include <stack>
#include <optional>
#include <thread>

using namespace std;

struct iodata
{
    uint64_t stat[7];

    iodata operator+=(iodata other)
    {
        iodata result;
        for(size_t j = 0; j < (sizeof(stat) / sizeof(*stat)); j++) {
            stat[j] += other.stat[j];
        }
        return result;
    }

    iodata operator-(iodata other) const
    {
        iodata result;
        for(size_t j = 0; j < (sizeof(stat) / sizeof(*stat)); j++) {
            result.stat[j] = stat[j] - other.stat[j];
        }
        return result;
    }
};

static optional<int> open_io(pid_t pid)
{
    int fd;
    char path[100];
    sprintf(path, "/proc/%d/io", pid);
    fd = open(path, O_RDONLY);

    if (fd == -1) {
        if (errno == ESRCH || errno == EPERM || errno == EACCES)
            return {};
        perror("open");
        fprintf(stderr, "Path: %s\n", path);
        exit(1);
    }
    return fd;
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

class ProcFsFiles
{
    vector<int> fds;

public:
    ProcFsFiles(auto begin, auto end)
    {
        for(auto it = begin; it != end; it++)
        {
            pid_t p = *it;
            auto fd = open_io(p);
            if (fd)
                fds.push_back(*fd);
        }
    }

    iodata read_iodata()
    {
        iodata result{};
        bool any = false;

        for (int fd : fds)
        {
            if (-1 == lseek(fd, 0, SEEK_SET))
            {
                if (errno == ESRCH)
                    continue;
                perror("lseek");
                exit(1);
            }

            char buffer[4096];
            ssize_t read_count = read(fd, buffer, sizeof(buffer) - 1);

            if (read_count == -1)
            {
                if (errno == ESRCH || errno == EACCES || errno == EPERM)
                    continue;
                perror("read");
                exit(1);
            }

            buffer[read_count] = 0;

            iodata d;
            int matched = sscanf(buffer,
                                 "rchar: %lu\nwchar: %lu\nsyscr: %lu\nsyscw: %lu\nread_bytes: %lu\nwrite_bytes: %lu\ncancelled_write_bytes: %lu\n",
                                 &d.stat[0], &d.stat[1], &d.stat[2], &d.stat[3], &d.stat[4], &d.stat[5], &d.stat[6]);

            if (matched != 7) {
                fprintf(stderr, "Reading format error!\n");
                exit(1);
            }

            result += d;
            any = true;
        }

        if (!any)
            exit(0);

        return result;
    }

    ~ProcFsFiles()
    {
        for(int fd : fds)
        {
            close(fd);
        }
    }
};

class ProcFsFiles_ThreadSafe
{
    mutex files_mutex;
    unique_ptr<ProcFsFiles> files;

public:
    void set(unique_ptr<ProcFsFiles> files)
    {
        lock_guard<mutex> lock(files_mutex);
        this->files = std::move(files);
    }

    iodata read_iodata()
    {
        lock_guard<mutex> lock(files_mutex);
        return files->read_iodata();
    }
};

static ProcFsFiles_ThreadSafe files;

static void read_and_print(int64_t pause_ns)
{
    struct timespec now;
    if (-1 == clock_gettime(CLOCK_MONOTONIC, &now))
    {
        perror("clock_gettime");
        exit(1);
    }

    timespec res;
    if (0 != clock_getres(CLOCK_MONOTONIC, &res))
    {
        perror("clock_getres");
        exit(1);
    }

    fprintf(stderr, "Resolution: %lu.%09lu\n", res.tv_sec, res.tv_nsec);

    iodata lastdata = files.read_iodata();

    for(;;)
    {
        now.tv_nsec += pause_ns;
        if (now.tv_nsec > 1000000000)
        {
            now.tv_nsec -= 1000000000;
            now.tv_sec++;
        }

        if (0 != clock_nanosleep(CLOCK_MONOTONIC, TIMER_ABSTIME, &now, NULL))
        {
            perror("clock_nanosleep");
            exit(1);
        }

        iodata data = files.read_iodata();
        iodata diff = data - lastdata;

        printf("%lu,%lu,%lu,%lu,%lu,%lu,%lu\n",
               diff.stat[0],
               diff.stat[1],
               diff.stat[2],
               diff.stat[3],
               diff.stat[4],
               diff.stat[5],
               diff.stat[6]);
        fflush(stdout);

        lastdata = data;
    }
}

int main(int argc, char **argv)
{
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <pause_ns>\n", argv[0]);
        exit(1);
    }

    int64_t pause_ns = atoi(argv[1]);

    if (pause_ns > 1000000000)
    {
        fprintf(stderr, "pause_ns must be below or equal to 1000000000!\n");
        exit(1);
    }

    string pids_str;
    getline(cin, pids_str);

    auto pids = parse_pids((char*)pids_str.c_str());

    if(pids.empty())
        exit(0);

    files.set(make_unique<ProcFsFiles>(pids.begin(), pids.end()));

    thread t(read_and_print, pause_ns);

    for(;;)
    {
        getline(cin, pids_str);
        auto pids = parse_pids((char*)pids_str.c_str());

        if(pids.empty())
            exit(0);

        files.set(make_unique<ProcFsFiles>(pids.begin(), pids.end()));
    }
}
