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
#include <string>
// ----------------------------------------------------------------------------------------------
#include <limits.h>
#include <stdio.h>
#include "../clinq/clinq.hpp"
// ----------------------------------------------------------------------------------------------
#define TEST_PRELUDE()                  test_prelude(__FILE__, __LINE__, __FUNCTION__)
#define TEST_ASSERT(expected, found)    test_assert(__FILE__, __LINE__, expected, #expected, found, #found, expected == found)
// ---------------------------------------------------------------------------------------------- 
using namespace clinq;
// ---------------------------------------------------------------------------------------------- 
namespace
{

    struct  customer
    {
        std::string     first_name  ;
        std::string     last_name   ;

        customer (std::string first_name, std::string last_name)
            :   first_name  (std::move (first_name))
            ,   last_name   (std::move (last_name))
        {
        }
    };

    template<typename TValueArray>
    int get_array_size (TValueArray & a)
    {
        return clinq::detail::get_array_properties<TValueArray>::size;
    }

    std::size_t         errors              = 0;
    
    std::vector<int>    empty               ;

    customer            customers[]         = 
        {
            customer ("Bill"    , "Gates"   ),
            customer ("Steve"   , "Jobs"    ),
            customer ("Richard" , "Stallman"),
            customer ("Linus"   , "Thorvals"),
        };
    int                 count_of_customers  = get_array_size(customers); 

    int                 ints[]              = {3,1,4,1,5,9,2,6,5,3,5,8,9,7,9,3,2,3,8,4,6,2,6,4,3,3,8,3,2,7,9,5};
    int                 count_of_ints       = get_array_size(ints); 

    void test_prelude (
            char const *    file
        ,   int             line_no
        ,   char const *    test
        )
    {
        printf(
                "%s(%d): RUNNING: %s\r\n"
            ,   file
            ,   line_no
            ,   test
            );
    }

    bool test_assert(
            char const *    file
        ,   int             line_no
        ,   bool            expected
        ,   char const *    expected_name
        ,   bool            found
        ,   char const *    found_name
        ,   bool            result
        )
    {
        if (!result)
        {
            ++errors;
            printf(
                    "%s(%d): ERROR_EXPECTED: %s(%s), FOUND: %s(%s)\r\n"
                ,   file
                ,   line_no
                ,   expected    ? "true" : "false"
                ,   expected_name
                ,   found       ? "true" : "false"
                ,   found_name
                );
        }

        return result;
    }

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

    void test_int_at (int index, int v)
    {
        if (
                TEST_ASSERT(true, index > -1)
            &&  TEST_ASSERT(true, index < count_of_ints)
            &&  TEST_ASSERT(ints[index], v)
            )
        {
        }
        else
        {
            printf("    @index:%d\r\n", index);
        }
    }

    void test_from ()
    {
        TEST_PRELUDE();

        {
            auto q = from(empty);
            auto index = 0;

            while (q.next())
            {
                test_int_at (index, q.front());
                ++index;
            }
            TEST_ASSERT(0, index);
        }
        {
            auto q = from_array(ints);

            auto index = 0;

            while (q.next())
            {
                test_int_at (index, q.front());
                ++index;
            }
            TEST_ASSERT(count_of_ints, index);
        }
    }

    void test_count ()
    {
        TEST_PRELUDE();

        {
            auto count_result = from(empty) >> count();
            TEST_ASSERT(0, count_result);
        }

        {
            auto count_result = from_array(ints) >> count();
            TEST_ASSERT(count_of_ints, (int)count_result);
        }
    }

    void test_first ()
    {
        TEST_PRELUDE();

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
        TEST_PRELUDE();

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

    void test_min ()
    {
        TEST_PRELUDE();

        {
            auto min_result = from(empty) >> min();
            TEST_ASSERT(INT_MAX, min_result);
        }

        {
            auto min_result = from_array(ints) >> min();
            TEST_ASSERT(1, min_result);
        }
    }

    void test_max ()
    {
        TEST_PRELUDE();

        {
            auto max_result = from(empty) >> max();
            TEST_ASSERT(INT_MIN, max_result);
        }

        {
            auto max_result = from_array(ints) >> max();
            TEST_ASSERT(9, max_result);
        }
    }

    void test_for_each ()
    {
        TEST_PRELUDE();

        {
            auto index = 0;
            from(empty) >> for_each([&](int i){test_int_at(index, i); ++index;});
            TEST_ASSERT(0, index);
        }

        {
            auto index = 0;
            from_array(ints) >> for_each([&](int i){test_int_at(index, i); ++index;});
            TEST_ASSERT(count_of_ints, index);
        }
    }
} 

int main()
{
    // -------------------------------------------------------------------------
    test_from       ();
    test_count      ();
    test_first      ();
    test_sum        ();
    test_min        ();
    test_for_each   ();
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

