#include <iostream>
#include <numeric>
#include <random>

using namespace std;

template<typename T>
class SmallVectorView
{
    size_t len = 0;
    T data[0];

public:
    void push_back(T elem)
    {
        data[len++] = elem;
    }

    [[nodiscard]] size_t size() const { return len; }

    T* begin() { return data; }
    T* end() { return data + len; }
};

template<typename T, size_t capacity>
class SmallVector : public SmallVectorView<T>
{
    T data[capacity];
};




template<typename T = uint64_t>
class Mod
{
    T n;
    using T2 = typename std::conditional<sizeof(T) == 8, __uint128_t, uint64_t>::type;

public:
    explicit Mod(T n) : n(n) {}

    [[nodiscard]] T mul(T a, T b) const
    {
        T2 a_ = a;
        T2 b_ = b;
        T2 res = a_ * b_;
        return res % n;
    }

    [[nodiscard]] T square(T x) const
    {
        return mul(x, x);
    }

    [[nodiscard]] T pow(T a, T p) const
    {
        if(p == 0)
            return 1;

        T res = pow(square(a), p / 2);

        if(p & 1)
            res = mul(res, a);

        return res;
    }

    bool check_witness(uint64_t a, uint64_t d, uint64_t s)
    {
        uint64_t x = pow(a, d);
        if(x == 1 || x == n - 1)
            return true;

        for(uint64_t r = 0; r + 1 < s; r++)
        {
            x = square(x);
            if(x == n - 1)
                return true;
        }
        return false;
    }
};

static const uint8_t witnesses[] = { 2, 3, 5, 7, 11, 13, 17, 19, 23, 29, 31, 37 };

template<typename T>
static bool check_witnesses(uint64_t n, uint64_t d, uint64_t s)
{
    for(uint8_t a : witnesses)
    {
        if(!Mod<T>(n).check_witness(a, d, s))
            return false;
    }

    return true;
}

static bool miller_rabin(uint64_t n)
{
    if(n < 38)
    {
        for(uint8_t witness : witnesses)
            if(n == witness)
                return true;

        return false;
    }

    uint64_t d = n - 1;
    uint64_t s = 0;
    while((d % 2) == 0)
    {
        s++;
        d /= 2;
    }

    if(n <= numeric_limits<uint32_t>::max())
        return check_witnesses<uint32_t>(n, d, s);
    else
        return check_witnesses<uint64_t>(n, d, s);
}






static uint64_t f_mod(uint64_t x, uint64_t c, uint64_t n)
{
    __uint128_t x_ = x;
    __uint128_t res = x_ * x_ + c;
    return res % n;
}

static uint64_t custom_gcd(uint64_t m, uint64_t n)
{
    if (m == 0)
	    return n;
    if (n == 0)
	    return m;

    const int i = std::__countr_zero(m);
    m >>= i;
    const int j = std::__countr_zero(n);
    n >>= j;
    const int k = i < j ? i : j; // min(i, j)

    while (true)
	{
#if MANUALLY_OPTIMIZED
        uint64_t tmp;
        asm("cmp %1, %0\n"
            "mov %0, %2\n"
            "cmova %1, %0\n"
            "cmova %2, %1"
            : "+r" (m), "+r"(n), "=&r" (tmp) :: "cc");
#else
        if (m > n)
        {
            uint64_t tmp = m;
            m = n;
            n = tmp;
        }
#endif

	    n -= m;

	    if (n == 0)
	        return m << k;

	    n >>= std::__countr_zero(n);
	}
}

static uint64_t get_divisor(uint64_t n, uint64_t x_0, uint64_t c)
{
    uint64_t x_s = x_0;
    uint64_t x_t = x_0;

    for(;;)
    {
        x_s = f_mod(x_s, c, n);
        x_t = f_mod(f_mod(x_t, c, n), c, n);

        uint64_t x_s_t;

        if(x_s == x_t)
            return 0; // Nothing found

        if(x_s >= x_t)
            x_s_t = x_s - x_t;
        else
            x_s_t = n - x_t + x_s;

        uint64_t candidate = custom_gcd(n, x_s_t);

        if(candidate > 1)
            return candidate;
    }
}

static std::mt19937 rng;

static uint64_t try_get_divisor(uint64_t n)
{
    std::uniform_int_distribution<uint64_t> dist(0, n - 1);
    uint64_t c = dist(rng);
    uint64_t x_0 = dist(rng);
    return get_divisor(n, x_0, c);
}

static uint64_t get_divisor(uint64_t n)
{
    uint64_t div;
    while((div = try_get_divisor(n)) == 0);
    return div;
}

static void find_prime_factors(SmallVectorView<uint64_t>& dst, uint64_t n)
{
    if(n <= 1)
        return;

    if(n == 2)
    {
        dst.push_back(2);
        return;
    }

    uint64_t a;
    uint64_t b;

    if(n % 2 == 0)
    {
        a = 2;
        b = n / 2;
    }
    else
    {
        if(miller_rabin(n))
        {
            dst.push_back(n);
            return;
        }

        a = get_divisor(n);
        b = n / a;
    }

    find_prime_factors(dst, a);
    find_prime_factors(dst, b);
}

static void solve(uint64_t n)
{
    SmallVector<uint64_t, 36> prime_factors;
    find_prime_factors(prime_factors, n);
    std::sort(prime_factors.begin(), prime_factors.end());
    for(auto prime : prime_factors)
        cout << prime << ' ';
    cout << '\n';
}

int main()
{
    iostream::sync_with_stdio(false);
    cin.tie(nullptr);

    uint_fast16_t t;
    cin >> t;

    for(uint_fast16_t i = 0; i < t; i++)
    {
        uint64_t n_i;
        cin >> n_i;
        solve(n_i);
    }
}
