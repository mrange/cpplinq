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
#include "../CppLinq/cpplinq.hpp"
// ----------------------------------------------------------------------------------------------
int main (int argc, char* args[])
{
    using namespace cpplinq;
    auto begin = args;
    auto end = args + argc;

    auto result = from_iterators (begin, end)
        >> skip (1)
        >> select ([] (char * cs) {return strlen (cs);})
        >> sum ()
        ;

    printf ("%d\n", result);

    return 0;
}
// ----------------------------------------------------------------------------------------------
