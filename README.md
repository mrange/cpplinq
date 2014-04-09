cpplinq - LINQ-like list manipulations for C++11
================================================

LINQ for C++ (cpplinq) is an extensible C++11 library of higher-order functions for range manipulation. cpplinq draws inspiration from LINQ for C#.

```cpp
#include "cpplinq.hpp"

int computes_a_sum ()
{
    using namespace cpplinq;    
    int ints[] = {3,1,4,1,5,9,2,6,5,4};

    // Computes the sum of all even numbers in the sequence above
    return 
            from_array (ints)
        >>  where ([](int i) {return i%2 ==0;})     // Keep only even numbers
        >>  sum ()                                  // Sum remaining numbers
        ;
}
```

See the documentation: http://cpplinq.codeplex.com/documentation


