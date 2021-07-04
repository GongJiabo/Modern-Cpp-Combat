#include <chrono>
#include <complex>
#include <iostream>
#include <string>
#include <thread>

using namespace std;

static constexpr double factors[] =
    {1.0, 1000.0, 1e-3, 1e-2, 0.0254, 0.3048, 0.9144, 1609.344};
    
struct length
{
    double value;
    enum unit
    {
        metre,
        kilometre,
        millimetre,
        centimetre,
        inch,
        foot,
        yard,
        mile
    };

    explicit length(double v, unit u = metre)
    {
        value = v * factors[u];
    }
};


length operator+(length lhs, length rhs)
{
    return length(lhs.value + rhs.value);
}

length operator""   _m(long double v)
{
    return length(v, length::metre);
}

length operator"" _cm(long double v)
{
    return length(v, length::centimetre);
}

int main()
{
    // 虚数类
    cout << "i * i = " << 1i * 1i << endl;
    
    // 自定义量
    length m1(1.0, length::metre);
    length m2 = m1 + 12.0_cm;
    return 0;
}