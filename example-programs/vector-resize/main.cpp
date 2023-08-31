#include <iostream>
#include <vector>
#include <numeric>

using namespace std;

// Add custom event tones for proving the hypothesis:
#define CUSTOM_SONIFICATION 1
#define OPTIMIZED_READ 1

#if CUSTOM_SONIFICATION
#include "osc.h"
#else
#define SEND_EVENT(...)
#endif


#if OPTIMIZED_READ
static uint64_t read_int()
{
    uint64_t res = 0;
    for (size_t digit; (digit = getchar_unlocked() - '0') < 10; res = res * 10 + digit);
    return res;
}
#else
static uint64_t read_int()
{
    uint64_t res;
    cin >> res;
    return res;
}
#endif

int main()
{
    iostream::sync_with_stdio(false);
    vector<uint64_t> data;

    size_t n = read_int();

    // Tell the vector how large our data array will have to be...
    data.reserve(n);

    for(size_t i = 0; i < n; i++)
    {
        uint64_t a = read_int();

        bool needs_increase = data.size() == data.capacity();

        if (needs_increase)
            SEND_EVENT("/custom1");

        // Dynamically sized array
        // Once it is full, three things happen:
        // 1. Allocation of larger buffer
        // 2. Copying of all existing data from the old buffer into the newly allocated
        // 3. Deallocation of the previous buffer

        // Can we hear it?

        // Yes!

        // Can we improve on it?

        data.push_back(a);

        if (needs_increase)
            SEND_EVENT("/custom2");
    }

    uint64_t sum = std::accumulate(data.begin(), data.end(), 0llu);
    cout << sum << endl;
}
