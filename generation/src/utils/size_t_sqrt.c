#include "size_t_sqrt.h"

size_t size_t_sqrt(size_t n)
{
    if (n < 2)
        return n;

    size_t s1 = n >> 1;
    size_t s2 = (s1 + n / s1) >> 1;
    while (s2 < s1)
    {
        s1 = s2;
        s2 = (s1 + n / s1) >> 1;
    }
    return s1;
}
