#include <iostream>
#include <span>
#include <bit>

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

static void do_linear(std::span<volatile uint8_t> mem)
{
    // Linear access through an array: Easily predictable -> no cache misses
    
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
    // Random access through an array: Not predictable -> Cache misses as soon as array gets too large to fit completely into the cache

    size_t mask = std::bit_floor(mem.size()) - 1;

    Random j;
    size_t i = 0;
    for(;;)
    {
        i += ++j & mask;
        if (i > mem.size())
            i -= mem.size();

        uint8_t val = mem[i];
    }
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <linear|random> <size>\n", argv[0]);
        exit(1);
    }

    std::string_view type = argv[1];
    size_t size = atoll(argv[2]);

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
        fprintf(stderr, "Unknown operation type \"%s\". Available options are:\nlinear\nrandom\n\n", argv[1]);
        exit(1);
    }

    std::span<volatile uint8_t> mem(new uint8_t[size]{}, size);
    fun(mem);
}
