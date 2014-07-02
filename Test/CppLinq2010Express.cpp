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
#include "../CppLinq/cpplinq.hpp"
// ----------------------------------------------------------------------------------------------
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

int main ()
{
    auto sum = computes_a_sum ();

    printf ("%d\r\n", sum);

    return 0;
}
// ----------------------------------------------------------------------------------------------
