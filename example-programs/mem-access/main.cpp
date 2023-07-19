#include <iostream>
#include <cstring>
#include <span>
#include <thread>
#include <csignal>
#include <functional>
#include <bit>
#include <utility>

class Random
{
    uint32_t x;

public:
    Random(uint32_t x = 0) : x(x) {}

    uint32_t operator++()
    {
        x = (uint64_t)x * 1664525 + 1013904223;
        return x;
    }
};

static void signal_handler(int sig)
{
    if(sig == SIGSTOP)
    {
        pthread_exit(nullptr);
    }
}

static void do_linear(std::span<volatile uint8_t> mem)
{

    //signal(SIGSTOP, signal_handler);

    for(;;)
    {
        for (size_t i = 0; i < mem.size(); i++)
        {
            uint8_t val = mem[i];
        }
    }
}

static void do_random(std::span<volatile uint8_t> mem)
{
    size_t mask = std::bit_ceil(mem.size()) - 1;

    Random j;
    for(;;)
    {
        size_t i = ++j & mask;
        if (i > mem.size())
            i -= mem.size();

        uint8_t val = mem[i];
    }
}

class CancelableThread
{
    static void* pthread_func(void* arg)
    {
        int state = PTHREAD_CANCEL_ASYNCHRONOUS;
        pthread_setcanceltype(state, &state);
        (*(std::function<void(void)>*)arg)();
        return (void*)nullptr;
    }

    std::function<void(void)> thread_func;
    pthread_t t;

public:
    explicit CancelableThread(std::function<void(void)> thread_func) : thread_func(std::move(thread_func))
    {
        int res = pthread_create(&t, nullptr, pthread_func, &this->thread_func);
        if (res != 0)
        {
            fprintf(stderr, "pthread_create: Error %d\n", res);
            exit(3);
        }
    }

    ~CancelableThread()
    {
        int res = pthread_cancel(t);
        if (res != 0)
        {
            fprintf(stderr, "pthread_cancel: Error %d\n", res);
            exit(4);
        }
    }
};

int main(int argc, char **argv)
{
    for(;;)
    {
        std::string type;
        size_t size;
        std::cin >> type >> size;

        std::unique_ptr<volatile uint8_t[]> m(new uint8_t[size]{});

        void (*fun)(std::span<volatile uint8_t> span);

        if(type == "linear")
        {
            fun = do_linear;
        }
        else if(type == "random")
        {
            fun = do_random;
        }
        else
        {
            fprintf(stderr, "Unknown operation type \"%s\". Available options are:\nlinear\nrandom\n\n", type.c_str());
            exit(1);
        }

        std::span<volatile uint8_t> mem(m.get(), size);
        std::function<void(void)> lambda = [fun, mem](){ fun(mem); };

        {
            CancelableThread t(lambda);
            sleep(1);
        }
    }
}
