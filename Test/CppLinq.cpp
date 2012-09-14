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
#include <map>
#include <numeric>
#include <string>
#include <vector>
// ----------------------------------------------------------------------------------------------
#include <limits.h>
#include <stdio.h>
#include "../CppLinq/cpplinq.hpp"
// ----------------------------------------------------------------------------------------------
#define TEST_PRELUDE()                  test_prelude(__FILE__, __LINE__, __FUNCTION__)
#define TEST_ASSERT(expected, found)    test_assert(__FILE__, __LINE__, expected, #expected, found, #found, expected == found)
// ---------------------------------------------------------------------------------------------- 
namespace
{

    struct  customer
    {
        std::size_t     id          ;
        std::string     first_name  ;
        std::string     last_name   ;

        customer (std::size_t id, std::string first_name, std::string last_name)
            :   id          (std::move(id))
            ,   first_name  (std::move (first_name))
            ,   last_name   (std::move (last_name))
        {
        }

        customer (customer const & c)
            :   id          (c.id)
            ,   first_name  (c.first_name)
            ,   last_name   (c.last_name)
        {
        }

        customer (customer && c)
            :   id          (std::move(c.id))
            ,   first_name  (std::move (c.first_name))
            ,   last_name   (std::move (c.last_name))
        {
        }
    };

    template<typename TValueArray>
    int get_array_size (TValueArray & a)
    {
        return cpplinq::detail::get_array_properties<TValueArray>::size;
    }

    int get_even_counts(int* is, int count)
    {
        auto c = 0;
        for (auto index = 0; index < count; ++index)
        {
            auto i = is[index];
            if (i%2 == 0)
            {
                ++c;
            }
        }
        return c;
    }

    std::size_t         errors              = 0;
    
    std::vector<int>    empty               ;

    customer            customers[]         = 
        {
            customer (1 , "Bill"    , "Gates"   ),
            customer (2 , "Steve"   , "Jobs"    ),
            customer (3 , "Richard" , "Stallman"),
            customer (4 , "Linus"   , "Thorvals"),
        };
    int                 count_of_customers  = get_array_size(customers); 

    int                 ints[]              = {3,1,4,1,5,9,2,6,5,3,5,8,9,7,9,3,2,3,8,4,6,2,6,4,3,3,8,3,2,7,9,5};
    int                 count_of_ints       = get_array_size(ints); 

    int                 even_count_of_ints  = get_even_counts (ints, count_of_ints);

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
        ,   std::string     expected
        ,   char const *    expected_name
        ,   std::string     found
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
                ,   expected.c_str()
                ,   expected_name
                ,   found.c_str()
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
        using namespace cpplinq;

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
        using namespace cpplinq;

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
        using namespace cpplinq;

        TEST_PRELUDE();

        {
            int first_result = from(empty) >> first();
            TEST_ASSERT(0, first_result);
        }

        {
            int first_result = from_array(ints) >> first();
            TEST_ASSERT(3, first_result);
        }
    }

    void test_sum ()
    {
        using namespace cpplinq;

        TEST_PRELUDE();

        {
            int sum_result = from(empty) >> sum();
            TEST_ASSERT(0, sum_result);
        }

        {
            int sum_of_ints = std::accumulate(ints, ints + count_of_ints, 0);
            int sum_result = from_array(ints) >> sum();
            TEST_ASSERT(sum_of_ints, sum_result);
        }
    }

    void test_min ()
    {
        using namespace cpplinq;

        TEST_PRELUDE();

        {
            int min_result = from(empty) >> min();
            TEST_ASSERT(INT_MAX, min_result);
        }

        {
            int min_result = from_array(ints) >> min();
            TEST_ASSERT(1, min_result);
        }
    }

    void test_max ()
    {
        using namespace cpplinq;

        TEST_PRELUDE();

        {
            int max_result = from(empty) >> max();
            TEST_ASSERT(INT_MIN, max_result);
        }

        {
            int max_result = from_array(ints) >> max();
            TEST_ASSERT(9, max_result);
        }
    }

    void test_for_each ()
    {
        using namespace cpplinq;

        TEST_PRELUDE();

        {
            int index = 0;
            from(empty) >> for_each([&](int i){test_int_at(index, i); ++index;});
            TEST_ASSERT(0, index);
        }

        {
            int index = 0;
            from_array(ints) >> for_each([&](int i){test_int_at(index, i); ++index;});
            TEST_ASSERT(count_of_ints, index);

        }
    }

    void test_to_vector ()
    {
        using namespace cpplinq;

        TEST_PRELUDE();

        {
            std::vector<int> to_vector_result = from (empty) >> to_vector();
            TEST_ASSERT(0, to_vector_result.size());
        }

        {
            std::vector<int> to_vector_result = from_array(ints) >> to_vector();
            TEST_ASSERT(count_of_ints, (int)to_vector_result.size());
            for(auto index = 0U; index < to_vector_result.size(); ++index)
            {
                test_int_at (index, to_vector_result[index]);
            }
        }
    }

    void test_to_map ()
    {
        using namespace cpplinq;

        TEST_PRELUDE();

        {
            std::map<int,int> to_map_result = from (empty) >> to_map([](int i){return i;});
            TEST_ASSERT(0, to_map_result.size());
        }

        {
            auto to_map_result = from_array (customers) >> to_map([](customer const & c){return c.id;});
            TEST_ASSERT(count_of_customers, (int)to_map_result.size());

            for(auto index = 0; index < count_of_customers; ++index)
            {
                auto c1 = customers[index];
                auto find_c2 = to_map_result.find(c1.id);
                if (TEST_ASSERT(true, (find_c2 != to_map_result.end())))
                {
                    auto c2 = find_c2->second;

                    if (
                            TEST_ASSERT((int)c1.id, (int)c2.id)
                        &&  TEST_ASSERT(c1.first_name, c2.first_name)
                        &&  TEST_ASSERT(c1.last_name, c2.last_name)
                        )
                    {
                    }
                    else
                    {
                        printf("    @index:%d\r\n", index);
                    }
                }
            }
        }
    }

    void test_container ()
    {
        using namespace cpplinq;

        TEST_PRELUDE();

        // TODO:
    }

    void test_where ()
    {
        using namespace cpplinq;

        TEST_PRELUDE();

        {
            auto c = from(empty) >> where ([](int i) {return i%2==0;}) >> count ();
            TEST_ASSERT(0, c);
        }
        {
            auto c = from_array(ints) >> where ([](int i) {return i%2==0;}) >> count ();
            TEST_ASSERT(even_count_of_ints, (int)c);
        }
    }

    void test_select ()
    {
        using namespace cpplinq;

        TEST_PRELUDE();

        // TODO:
    }

    void test_orderby ()
    {
        using namespace cpplinq;

        TEST_PRELUDE();

        // TODO:
    }

    void test_skip ()
    {
        using namespace cpplinq;

        TEST_PRELUDE();

        {
            auto q = from(empty) >> skip (5);
            auto index = 0;

            while (q.next())
            {
                test_int_at (index, q.front());
                ++index;
            }
            TEST_ASSERT(0, index);
        }
        {
            auto q = from_array(ints) >> skip (5);

            auto index = 5;

            while (q.next())
            {
                test_int_at (index, q.front());
                ++index;
            }
            TEST_ASSERT(count_of_ints, index);
        }
    }

    void test_take ()
    {
        using namespace cpplinq;

        TEST_PRELUDE();

        {
            auto q = from(empty) >> take (5);
            auto index = 0;

            while (q.next())
            {
                test_int_at (index, q.front());
                ++index;
            }
            TEST_ASSERT(0, index);
        }
        {
            auto q = from_array(ints) >> take (5);

            auto index = 0;

            while (q.next())
            {
                test_int_at (index, q.front());
                ++index;
            }
            TEST_ASSERT(5, index);
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
    test_to_vector  ();
    test_to_map     ();
    test_where      ();
    test_take       ();
    test_skip       ();
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

