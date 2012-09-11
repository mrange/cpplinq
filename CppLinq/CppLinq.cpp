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
#include <algorithm>
#include <numeric>
// ----------------------------------------------------------------------------------------------
#include <limits.h>
#include <stdio.h>
#include "../clinq/clinq.hpp"
// ----------------------------------------------------------------------------------------------
#define TEST_ASSERT(expected, found)    test_assert(__FILE__, __LINE__, expected, #expected, found, #found, expected == found)
// ---------------------------------------------------------------------------------------------- 
using namespace clinq;
// ---------------------------------------------------------------------------------------------- 
namespace
{

    std::size_t         errors          = 0;
    
    std::vector<int>    empty           ;

    int                 ints[]          = {3,1,4,1,5,9,2,6,5,3,5,8,9,7,9,3,2,3,8,4,6,2,6,4,3,3,8,3,2,7,9,5};
    std::size_t         count_of_ints   = sizeof(ints)/sizeof(ints[0]); 

    bool test_assert(
            char const *    file
        ,   int             line_no
        ,   int             expected
        ,   char const *    expected_name
        ,   int             found
        ,   char const *    found_name
        ,   bool            result
        )
    {
        if (!result)
        {
            ++errors;
            printf(
                    "%s(%d): ERROR_EXPECTED: %d(%s), FOUND: %d(%s)\r\n"
                ,   file
                ,   line_no
                ,   expected
                ,   expected_name
                ,   found
                ,   found_name
                );
        }

        return result;
    }

    void test_from_array ()
    {
        auto q = from_array(ints);

        auto index = 0;

        while (q.next())
        {
            if (!TEST_ASSERT(ints[index], q.front()))
            {
                printf("    @index:%d\r\n", index);
            }
            ++index;
        }
    }

    void test_count ()
    {
        {
            auto count_result = from(empty) >> count();
            TEST_ASSERT(0, count_result);
        }

        {
            auto count_result = from_array(ints) >> count();
            TEST_ASSERT(count_of_ints, count_result);
        }
    }

    void test_first ()
    {
        {
            auto first_result = from(empty) >> first();
            TEST_ASSERT(0, first_result);
        }

        {
            auto first_result = from_array(ints) >> first();
            TEST_ASSERT(3, first_result);
        }
    }

    void test_sum ()
    {
        {
            auto sum_result = from(empty) >> sum();
            TEST_ASSERT(0, sum_result);
        }

        {
            auto sum_of_ints = std::accumulate(ints, ints + count_of_ints, 0);
            auto sum_result = from_array(ints) >> sum();
            TEST_ASSERT(sum_of_ints, sum_result);
        }
    }
} 

int main()
{
    // -------------------------------------------------------------------------
    test_from_array ();
    test_count      ();
    test_first      ();
    test_sum        ();
    // -------------------------------------------------------------------------
    if (errors == 0)
    {
        printf ("PASS\r\n");
    }
    else
    {
        printf ("FAIL\r\n");
    }
    // -------------------------------------------------------------------------
    return errors > 0 ? 101 : 0;
    // -------------------------------------------------------------------------
}
// ---------------------------------------------------------------------------- -

