// ----------------------------------------------------------------------------------------------
// Copyright (c) Mårten Rånge.
// ----------------------------------------------------------------------------------------------
// This source code is subject to terms and conditions of the Microsoft Public License. A 
// copy of the license can be found in the License.html file at the root of this distribution. 
// If you cannot locate the  Microsoft Public License, please send an email to 
// dlr@microsoft.com. By using this source code in any fashion, you are agreeing to be bound 
//  by the terms of the Microsoft Public License.
// ----------------------------------------------------------------------------------------------
// You must not remove this notice, or any other, from this software.
// ----------------------------------------------------------------------------------------------
#include "stdafx.h"
// ----------------------------------------------------------------------------------------------
#pragma warning(disable:4100)
// ----------------------------------------------------------------------------------------------
#undef min
#undef max
// ----------------------------------------------------------------------------------------------
#include <limits.h>
#include <stdio.h>
#include "../clinq/clinq.hpp"
// ----------------------------------------------------------------------------------------------
using namespace clinq;
// ---------------------------------------------------------------------------------------------- 
namespace
{
    bool is_prime (int i)
    {
        if (i < 2)
        {
            return false;
        }
        else if (i == 2)
        {
            return true;
        }
        else
        {
            auto r = (int)std::ceil(std::sqrt(i));

            for (auto iter = 2; iter <= r; ++iter)
            {
                if (i % iter == 0)
                {
                    return false;
                }
            }

            return true;
        }
    }

    std::vector<int> some_primes(size_t how_many)
    {
        auto xs = 
                range (0, INT_MAX)
            >>  where (is_prime)
            >>  take  (how_many)
            >>  to_vector ()
            ;

        return xs;
    }
} 
int main()
{
    printf("Hi\r\n");

    int ints[]  = {3,1,4,1,5,9,2,6,5,3,5,8,9,7,9,3,2,3,8,4,6,2,6,4,3,3,8,3,2,7,9,5};
    int count   = sizeof(ints)/sizeof(ints[0]); 

    std::vector<int> container (ints, ints + count);

    auto q = 
//            range (0, 100)
//            from_iterators (ints, ints + count)
//            from (container)
            from_array (ints)
        >>  skip(2U)
        >>  take(8U)
        >>  where([](int i){return i%2 == 0;})
        >>  select([](int i) -> short {return (short)i + 1;})
        >>  orderby([](short s){return s;})
        >>  thenby([](short s){return s;})
        ;
    
    auto v  = q >> to_vector();
    auto d  = q >> to_map([](short s) {return s;});
    auto s = q >> sum();
    auto m = q >> max();
    auto m2 = q >> min();
    auto c = q >> clinq::count(); 
    auto f = q >> first();
    auto cont = q >> clinq::container ();

    std::for_each(
            cont.begin()
        ,   cont.end()
        ,   [](short s) {printf("I1: %d\r\n",s);}
        );

    q >> for_each([](short s) {printf("I0: %d\r\n",s);});

    for (auto i : v)
    {
        printf("I2: %d\r\n",i);
    }

    for (auto i : d)
    {
        printf("I3: %d,%d\r\n",i.first, i.second);
    }

    printf("Sum: %d\r\n", s);
    printf("Max: %d\r\n", m);
    printf("Min: %d\r\n", m2);
    printf("Count: %d\r\n", c);
    printf("First: %d\r\n", f);

    auto primes = some_primes(10U);
    for (auto prime : primes)
    {
        printf("Prime: %d\r\n", prime);
    }


   return 0;
}

