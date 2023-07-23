#include <iostream>
#include <vector>
#include <numeric>

using namespace std;

#define CUSTOM_SONIFICATION 0
#define OPTIMIZED_READ 1

#if CUSTOM_SONIFICATION
#include <osc.h>
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

    //data.reserve(n);

    for(size_t i = 0; i < n; i++)
    {
        uint64_t a = read_int();

        bool needs_increase = data.size() == data.capacity();

        if (needs_increase)
        {
            SEND_EVENT("/custom1");
        }

        data.push_back(a);

        if (needs_increase)
        {
            SEND_EVENT("/custom2");
        }
    }

    uint64_t sum = std::accumulate(data.begin(), data.end(), 0llu);
    cout << sum << endl;
}
