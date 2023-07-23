#include <iostream>
#include <bit>
#include <span>
#include <memory>

static std::pair<uint32_t, uint32_t> count_branchless(std::span<const unsigned int> numbers)
{
    size_t mask = std::bit_floor(numbers.size()) - 1;

    uint64_t tk = 0;
    for (unsigned int n : numbers)
    {
        n = numbers[n & mask];

        tk += 1 + (uint64_t)(0u - (n % 2));
    }
    return {tk >> 32, tk & UINT32_MAX};
}

static std::pair<uint32_t, uint32_t> count_branching(std::span<const unsigned int> numbers)
{
    size_t mask = std::bit_floor(numbers.size()) - 1;

    uint32_t taken = 0, nontaken = 0;
    for (unsigned int n : numbers)
    {
        n = numbers[n & mask];

        if (n % 2)
        {
            taken++;
        }
        else
        {
            nontaken++;
        }
    }
    return {taken, nontaken};
}

static std::pair<uint32_t, uint32_t> count_alternating(std::span<const unsigned int> numbers)
{
    size_t shift = 27 - std::bit_width(numbers.size());

    static int index = 0;
    index++;
    if ((index >> shift) & 1)
        return count_branchless(numbers);
    else
        return count_branching(numbers);
}

static void print_usage(const char *prog_name)
{
    std::cerr << "Usage: " << prog_name << " <size> branching|branchless|alternating" << std::endl;
    exit(1);
}

int main(int argc, char **argv)
{
    if (argc < 3)
        print_usage(argv[0]);

    size_t numbers_size = atoll(argv[1]);
    std::string_view cmd = argv[2];
    std::pair<uint32_t, uint32_t> (*func)(std::span<const unsigned int>);

    if (cmd == "branching")
        func = count_branching;
    else if (cmd == "branchless")
        func = count_branchless;
    else if (cmd == "alternating")
        func = count_alternating;
    else
        print_usage(argv[0]);

    std::span<unsigned int> numbers(new unsigned int[numbers_size], numbers_size);
    std::cin.read(reinterpret_cast<char*>(numbers.data()), numbers.size_bytes());

    uint64_t taken = 0, nontaken = 0;
    size_t N = (1 << 29) / numbers.size();
    for(size_t i = 0; i < N; i++)
    {
        auto [taken0, nontaken0] = func(numbers);
        taken += taken0;
        nontaken += nontaken0;
    }

    std::cerr << "Taken: " << taken << ", Nontaken: " << nontaken << std::endl;
}
