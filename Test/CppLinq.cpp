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
#ifdef _MSC_VER 
#   pragma warning (disable:4100)
#   pragma warning (disable:4996)
#endif
// ----------------------------------------------------------------------------------------------
#undef min
#undef max
// ----------------------------------------------------------------------------------------------
#include <algorithm>
#include <chrono>
#include <map>
#include <numeric>
#include <string>
#include <vector>
// ----------------------------------------------------------------------------------------------
#include <limits.h>
#include <stdio.h>
// ----------------------------------------------------------------------------------------------
#include "../CppLinq/cpplinq.hpp"
// ----------------------------------------------------------------------------------------------
#define TEST_PRELUDE()                  test_prelude(__FILE__, __LINE__, __FUNCTION__)
#define TEST_ASSERT(expected, found)    test_assert(__FILE__, __LINE__, expected, #expected, found, #found, (expected == found))
// ---------------------------------------------------------------------------------------------- 
namespace
{

    struct  customer
    {
        std::size_t     id          ;
        std::string     first_name  ;
        std::string     last_name   ;

        customer (std::size_t id, std::string first_name, std::string last_name)
            :   id          (std::move (id))
            ,   first_name  (std::move (first_name))
            ,   last_name   (std::move (last_name))
        {
        }

        customer & operator= (customer const & c)
        {
            if (std::addressof (c) == this)
            {
                return *this;
            }

            id          = c.id          ;
            first_name  = c.first_name  ;
            last_name   = c.last_name   ;

            return *this;
        }

        customer (customer const & c)
            :   id          (c.id)
            ,   first_name  (c.first_name)
            ,   last_name   (c.last_name)
        {
        }

        customer (customer && c)
            :   id          (std::move (c.id))
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

    int get_even_counts (int* is, int count)
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
            customer (4 , "Linus"   , "Torvalds"),

            customer (11, "Steve"   , "Ballmer" ),
            customer (12, "Tim"     , "Cook"    ),

            customer (21, "Melinda" , "Gates"   ),
        };
    int const           count_of_customers  = get_array_size (customers); 

    int                 ints[]              = {3,1,4,1,5,9,2,6,5,3,5,8,9,7,9,3,2,3,8,4,6,2,6,4,3,3,8,3,2,7,9,5};
    int const           count_of_ints       = get_array_size (ints); 

    int const           even_count_of_ints  = get_even_counts (ints, count_of_ints);

    void test_prelude (
            char const *    file
        ,   int             line_no
        ,   char const *    test
        )
    {
        printf (
                "%s(%d): RUNNING: %s\r\n"
            ,   file
            ,   line_no
            ,   test
            );
    }

    bool test_assert (
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
            printf (
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

    bool test_assert (
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
            printf (
                    "%s(%d): ERROR_EXPECTED: %s(%s), FOUND: %s(%s)\r\n"
                ,   file
                ,   line_no
                ,   expected.c_str ()
                ,   expected_name
                ,   found.c_str ()
                ,   found_name
                );
        }

        return result;
    }

    bool test_assert (
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
            printf (
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
                TEST_ASSERT (true, (index > -1))
            &&  TEST_ASSERT (true, (index < count_of_ints))
            &&  TEST_ASSERT (ints[index], v)
            )
        {
        }
        else
        {
            printf ("    @index:%d\r\n", index);
        }
    }

    void test_opt ()
    {
        using namespace cpplinq::detail;

        TEST_PRELUDE ();

        {
            opt<int> o;
            TEST_ASSERT (false, o.has_value ());
            TEST_ASSERT (false, o);

        }
        {
            opt<int> o1;
            opt<int> o2 (10);
            TEST_ASSERT (false, o1.has_value ());
            TEST_ASSERT (false, o1);
            TEST_ASSERT (true, o2.has_value ());
            TEST_ASSERT (true, (bool)o2);
            TEST_ASSERT (10, *o2);

            opt<int> o3 (o2);
            opt<int> o4 (o1);
            o3.swap (o4);
            TEST_ASSERT (false, o3.has_value ());
            TEST_ASSERT (false, o3);
            TEST_ASSERT (true, o4.has_value ());
            TEST_ASSERT (true, (bool)o4);
            TEST_ASSERT (10, *o4);

            o1.swap (o2);
            TEST_ASSERT (true, o1.has_value ());
            TEST_ASSERT (true, (bool)o1);
            TEST_ASSERT (10, *o1);
            TEST_ASSERT (false, o2.has_value ());
            TEST_ASSERT (false, o2);

            o2 = o1;
            TEST_ASSERT (true, o1.has_value ());
            TEST_ASSERT (true, (bool)o1);
            TEST_ASSERT (10, *o1);
            TEST_ASSERT (true, o2.has_value ());
            TEST_ASSERT (true, (bool)o2);
            TEST_ASSERT (10, *o2);

            o1 = 11;
            o2 = 12;
            TEST_ASSERT (true, o1.has_value ());
            TEST_ASSERT (true, (bool)o1);
            TEST_ASSERT (11, *o1);
            TEST_ASSERT (true, o2.has_value ());
            TEST_ASSERT (true, (bool)o2);
            TEST_ASSERT (12, *o2);

        }
        {
            opt<std::string> o1;
            opt<std::string> o2 ("Test");

            TEST_ASSERT (false, o1.has_value ());
            TEST_ASSERT (false, o1);
            TEST_ASSERT (true, o2.has_value ());
            TEST_ASSERT (true, (bool)o2);
            TEST_ASSERT ("Test", *o2);
            TEST_ASSERT (4, (int)o2->size ());
        
            o1 = "Test2";
            o2 = "Test3";
            TEST_ASSERT (true, o1.has_value ());
            TEST_ASSERT (true, (bool)o1);
            TEST_ASSERT ("Test2", *o1);
            TEST_ASSERT (true, o2.has_value ());
            TEST_ASSERT (true, (bool)o2);
            TEST_ASSERT ("Test3", *o2);

            o1.swap(o2);
            TEST_ASSERT (true, o1.has_value ());
            TEST_ASSERT (true, (bool)o1);
            TEST_ASSERT ("Test3", *o1);
            TEST_ASSERT (true, o2.has_value ());
            TEST_ASSERT (true, (bool)o2);
            TEST_ASSERT ("Test2", *o2);
        }
    }

    void test_from ()
    {
        using namespace cpplinq;

        TEST_PRELUDE ();

        {
            auto q = from (empty);

            typedef decltype (q.front ())   return_type;
            static_assert (
                    !std::is_reference<return_type>::value 
                ,   "front () must return non-reference when value_type = int"
                );

            auto index = 0;

            while (q.next ())
            {
                test_int_at (index, q.front ());
                ++index;
            }
            TEST_ASSERT (0, index);
        }
        {
            auto q = from_array (ints);

            typedef decltype (q.front ())   return_type;
            static_assert (
                    !std::is_reference<return_type>::value 
                ,   "front () must return non-reference when value_type = int"
                );

            auto index = 0;

            while (q.next ())
            {
                test_int_at (index, q.front ());
                ++index;
            }
            TEST_ASSERT (count_of_ints, index);
        }
        {
            auto q = from_copy (empty);

            typedef decltype (q.front ())   return_type;
            static_assert (
                    !std::is_reference<return_type>::value 
                ,   "front () must return non-reference when value_type = int"
                );

            auto index = 0;

            while (q.next ())
            {
                test_int_at (index, q.front ());
                ++index;
            }
            TEST_ASSERT (0, index);
        }
        {
            std::vector<int> is (ints, ints + count_of_ints);
            auto q = from_copy (is);

            typedef decltype (q.front ())   return_type;
            static_assert (
                    !std::is_reference<return_type>::value 
                ,   "front () must return non-reference when value_type = int"
                );

            auto index = 0;

            while (q.next ())
            {
                test_int_at (index, q.front ());
                ++index;
            }
            TEST_ASSERT (count_of_ints, index);
        }
        {
            auto q = from_array (customers);
            typedef decltype (q.front ())   return_type;
            static_assert (
                    std::is_reference<return_type>::value 
                ,   "front () must return non-reference when value_type = customer"
                );
        }
        {
            std::vector<customer> cs;
            auto q = from_copy (cs);
            typedef decltype (q.front ())   return_type;
            static_assert (
                    std::is_reference<return_type>::value 
                ,   "front () must return non-reference when value_type = customer"
                );
        }
    }

    void test_range ()
    {
        using namespace cpplinq;

        TEST_PRELUDE ();

        {
            auto start = 12;
            auto count = 10;
            auto index = start;

            auto q = range (start, count);

            typedef decltype (q.front ())   return_type;
            static_assert (
                    !std::is_reference<return_type>::value 
                ,   "front () must return non-reference when value_type = int"
                );

            while (q.next ())
            {
                if (!TEST_ASSERT (index, q.front ()))
                {
                        printf ("    @index:%d\r\n", index);
                }
                ++index;
            }
            TEST_ASSERT (start + count , index);
        }
    }

    void test_count ()
    {
        using namespace cpplinq;

        TEST_PRELUDE ();

        {
            auto count_result = from (empty) >> count ();
            TEST_ASSERT (0, (int)count_result);
        }

        {
            auto count_result = from_array (ints) >> count ();
            TEST_ASSERT (count_of_ints, (int)count_result);
        }
    }

    void test_first ()
    {
        using namespace cpplinq;

        TEST_PRELUDE ();

        {
            int first_result = from (empty) >> first ();
            TEST_ASSERT (0, first_result);
        }

        {
            int first_result = from_array (ints) >> first ();
            TEST_ASSERT (3, first_result);
        }
    }

    void test_sum ()
    {
        using namespace cpplinq;

        TEST_PRELUDE ();

        {
            int sum_result = from (empty) >> sum ();
            TEST_ASSERT (0, sum_result);
        }

        {
            int sum_of_ints = std::accumulate (ints, ints + count_of_ints, 0);
            int sum_result = from_array (ints) >> sum ();
            TEST_ASSERT (sum_of_ints, sum_result);
        }
    }

    void test_min ()
    {
        using namespace cpplinq;

        TEST_PRELUDE ();

        {
            int min_result = from (empty) >> min ();
            TEST_ASSERT (INT_MAX, min_result);
        }

        {
            int min_result = from_array (ints) >> min ();
            TEST_ASSERT (1, min_result);
        }
    }

    void test_max ()
    {
        using namespace cpplinq;

        TEST_PRELUDE ();

        {
            int max_result = from (empty) >> max ();
            TEST_ASSERT (INT_MIN, max_result);
        }

        {
            int max_result = from_array (ints) >> max ();
            TEST_ASSERT (9, max_result);
        }
    }

    void test_concatenate ()
    {
        using namespace cpplinq;

        TEST_PRELUDE ();

        {
            std::wstring concatenate_result = 
                    from (empty) 
                >>  select ([] (int i){return std::wstring ();})
                >>  concatenate (L"")
                ;
            TEST_ASSERT (true, concatenate_result.empty ());
        }

        {
            std::string concatenate_result = 
                    from_array (customers) 
                >>  select ([](customer const & c){return c.last_name;})
                >>  concatenate (", ")
                ;

            TEST_ASSERT ("Gates, Jobs, Stallman, Torvalds, Ballmer, Cook, Gates", concatenate_result);
        }
    }

    void test_for_each ()
    {
        using namespace cpplinq;

        TEST_PRELUDE ();

        {
            int index = 0;
            from (empty) >> for_each ([&](int i){test_int_at (index, i); ++index;});
            TEST_ASSERT (0, index);
        }

        {
            int index = 0;
            from_array (ints) >> for_each ([&](int i){test_int_at (index, i); ++index;});
            TEST_ASSERT (count_of_ints, index);

        }
    }

    void test_to_vector ()
    {
        using namespace cpplinq;

        TEST_PRELUDE ();

        {
            std::vector<int> to_vector_result = from (empty) >> to_vector ();
            TEST_ASSERT (0, (int)to_vector_result.size ());
        }

        {
            std::vector<int> to_vector_result = from_array (ints) >> to_vector ();
            TEST_ASSERT (count_of_ints, (int)to_vector_result.size ());
            for (auto index = 0U; index < to_vector_result.size (); ++index)
            {
                test_int_at (index, to_vector_result[index]);
            }
        }
    }

    void test_to_map ()
    {
        using namespace cpplinq;

        TEST_PRELUDE ();

        {
            std::map<int,int> to_map_result = from (empty) >> to_map ([](int i){return i;});
            TEST_ASSERT (0, (int)to_map_result.size ());
        }

        {
            auto to_map_result = from_array (customers) >> to_map ([](customer const & c){return c.id;});
            TEST_ASSERT (count_of_customers, (int)to_map_result.size ());

            for (auto index = 0; index < count_of_customers; ++index)
            {
                auto c1 = customers[index];
                auto find_c2 = to_map_result.find (c1.id);
                if (TEST_ASSERT (true, (find_c2 != to_map_result.end ())))
                {
                    auto c2 = find_c2->second;

                    if (
                            TEST_ASSERT ((int)c1.id, (int)c2.id)
                        &&  TEST_ASSERT (c1.first_name, c2.first_name)
                        &&  TEST_ASSERT (c1.last_name, c2.last_name)
                        )
                    {
                    }
                    else
                    {
                        printf ("    @index:%d\r\n", index);
                    }
                }
            }
        }
    }

    void test_container ()
    {
        using namespace cpplinq;
        using namespace cpplinq::experimental;

        TEST_PRELUDE ();

        {
            auto container_result = from (empty) >> container ();
            std::vector<int> v (container_result.begin (), container_result.end ());
            TEST_ASSERT (0, (int)v.size ());
        }

        // Container is experimental and doesn't have the right semantics just yet
        {
            auto container_result = from_iterators (ints, ints + count_of_ints) >> container ();
            std::vector<int> v (container_result.begin (), container_result.end ());
            if (TEST_ASSERT (count_of_ints, (int)v.size ()))
            {
                for (auto index = 0; index < count_of_ints; ++index)
                {
                    test_int_at (index, v[index]);
                }
            }

        }
    }

    void test_where ()
    {
        using namespace cpplinq;

        TEST_PRELUDE ();

        {
            auto c = from (empty) >> where ([](int i) {return i%2==0;}) >> count ();
            TEST_ASSERT (0, (int)c);
        }
        {
            auto c = from_array (ints) >> where ([](int i) {return i%2==0;}) >> count ();
            TEST_ASSERT (even_count_of_ints, (int)c);
        }
    }

    void test_select ()
    {
        using namespace cpplinq;

        TEST_PRELUDE ();

        {
            std::vector<double> select_result = from (empty) >> select ([](int i){return 1.0*i;}) >> to_vector ();
            TEST_ASSERT (0, (int)select_result.size ());
        }

        {
            std::vector<std::size_t> select_result = 
                    from_array (customers) 
                >>  select ([](customer const & c){return c.id;}) 
                >>  to_vector ()
                ;

            auto index = 0U;
            for (auto sz : select_result)
            {
                if (!TEST_ASSERT ((int)customers[index].id, (int)sz))
                {
                    printf ("    @index:%d\r\n", index);
                }

                ++index;
            }

            TEST_ASSERT (count_of_customers, (int)select_result.size ());
        }

    }

    void test_select_many ()
    {
        using namespace cpplinq;

        TEST_PRELUDE ();

        {
            std::vector<char> select_many_result = 
                    from_iterators (customers, customers) 
                >>  select_many ([](customer const & c){return from (c.last_name);}) 
                >>  to_vector ()
                ;

            TEST_ASSERT (0, (int)select_many_result.size ());
        }
        {
            std::vector<char> expected; 
            for (auto customer : customers)
            {
                expected.insert (
                        expected.end ()
                    ,   customer.last_name.begin ()
                    ,   customer.last_name.end ()
                    );
            }

            std::vector<char> select_many_result = 
                    from_array (customers) 
                >>  select_many ([](customer const & c){return from (c.last_name);}) 
                >>  to_vector ()
                ;

            if (TEST_ASSERT ((int)expected.size (), (int)select_many_result.size ()))
            {
                for (auto index = 0U; index < expected.size (); ++index)
                {
                    if (!TEST_ASSERT (expected[index], select_many_result[index]))
                    {
                        printf ("    @index:%d\r\n", index);
                    }
                }
            }
        }

    }

    void test_orderby ()
    {
        using namespace cpplinq;

        TEST_PRELUDE ();

        {
            auto c = from (empty) >> orderby_ascending ([](int i){return i;}) >> count ();
            TEST_ASSERT (0, (int)c);
        }

        const int test_set_size = 7;

        auto verify = [=](
                int expected[test_set_size]
            ,   std::vector<customer> const & sequence
            )
        {
            auto sz = sequence.size ();
            if (TEST_ASSERT (test_set_size, (int)sz))
            {
                auto index = 0U;
                for (auto c : sequence)
                {
                    if (!TEST_ASSERT (expected[index], (int)c.id))
                    {
                        printf ("    @index:%d\r\n", index);
                    }

                    ++index;
                }

            }
        };

        {
            int expected[] = 
                {
                    11,
                    12,
                    1,
                    21,
                    2,
                    3,
                    4,
                };

            auto sequence = 
                    from_array (customers) 
                >>  orderby_ascending ([] (customer const & c) {return c.last_name;})
                >>  thenby_ascending ([] (customer const & c) {return c.first_name;})
                >>  to_vector ()
                ;

            verify (expected, sequence);
        }
        {
            int expected[] = 
                {
                    4,
                    3,
                    2,
                    21,
                    1,
                    12,
                    11,
                };

            auto sequence = 
                    from_array (customers) 
                >>  orderby_descending ([] (customer const & c) {return c.last_name;})
                >>  thenby_descending ([] (customer const & c) {return c.first_name;})
                >>  to_vector ()
                ;

            verify (expected, sequence);
        }
        {
            int expected[] = 
                {
                    11,
                    12,
                    21,
                    1,
                    2,
                    3,
                    4,
                };

            auto sequence = 
                    from_array (customers) 
                >>  orderby ([] (customer const & c) {return c.last_name;}, true)
                >>  thenby ([] (customer const & c) {return c.first_name;}, false)
                >>  to_vector ()
                ;

            verify (expected, sequence);
        }
    }

    void test_skip ()
    {
        using namespace cpplinq;

        TEST_PRELUDE ();

        {
            auto q = from (empty) >> skip (5);
            auto index = 0;

            while (q.next ())
            {
                test_int_at (index, q.front ());
                ++index;
            }
            TEST_ASSERT (0, index);
        }
        {
            auto q = from_array (ints) >> skip (5);

            auto index = 5;

            while (q.next ())
            {
                test_int_at (index, q.front ());
                ++index;
            }
            TEST_ASSERT (count_of_ints, index);
        }
    }

    void test_take ()
    {
        using namespace cpplinq;

        TEST_PRELUDE ();

        {
            auto q = from (empty) >> take (5);
            auto index = 0;

            while (q.next ())
            {
                test_int_at (index, q.front ());
                ++index;
            }
            TEST_ASSERT (0, index);
        }
        {
            auto q = from_array (ints) >> take (5);

            auto index = 0;

            while (q.next ())
            {
                test_int_at (index, q.front ());
                ++index;
            }
            TEST_ASSERT (5, index);
        }
    }

    template<typename TPredicate>
    long long execute_testruns (
            std::size_t test_runs
        ,   TPredicate predicate
        )
    {
        auto then = std::chrono::high_resolution_clock::now ();

        for (auto test_run = 0U; test_run < test_runs; ++test_run)
        {
            predicate ();
        }

        auto now = std::chrono::high_resolution_clock::now ();

        auto diff = now - then;

        auto diff_in_ms = std::chrono::duration_cast<std::chrono::milliseconds>(diff).count ();

        return diff_in_ms;
    }

    void test_performance_sum ()
    {
        using namespace cpplinq;

        TEST_PRELUDE ();

        int         const test_repeat   = 80000     ;
        int         const test_size     = 20000     ;

        srand (19740531);

        auto test_set = 
                range (0, test_size)
            >>  select ([] (int i){return rand ();})
            >>  to_vector (test_size)
            ;

        auto expected_complete_sum = 0;
        auto expected = execute_testruns (
                test_repeat
            ,   [&] ()
                {
                    auto set_sum = 0;
                    for (auto v : test_set)
                    {
                        set_sum += v;
                    }
                    expected_complete_sum += set_sum;
                }
            );

        auto result_complete_sum = 0;
        auto result = execute_testruns (
                test_repeat
            ,   [&] ()
                {
                    auto set_sum =
                            from (test_set)
                        >>  sum ()
                        ;
                    result_complete_sum += set_sum;
                }                 
            );

        printf ("Expected sum: %d\r\n", expected_complete_sum);
        printf ("Result sum: %d\r\n", result_complete_sum);

        auto ratio_limit    = 3.0; 
        auto ratio          = ((double)expected)/result;
        TEST_ASSERT (true, (ratio > 1/ratio_limit && ratio < ratio_limit));
        printf (
                "Performance numbers for simple sum over numbers, expected:%d, result:%d, ratio_limit:%f, ratio:%f\r\n"
            ,   (int)expected
            ,   (int)result
            ,   ratio_limit
            ,   ratio
            );
    }

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
            auto r = (int)std::ceil (std::sqrt (i));

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

    void test_performance_is_prime ()
    {
        using namespace cpplinq;

        TEST_PRELUDE ();

#if _DEBUG
        int         const test_repeat   = 25        ;
#else
        int         const test_repeat   = 100       ;
#endif
        int         const test_size     = 10000     ;

        auto expected = execute_testruns (
                test_repeat
            ,   [&] ()
                {
                    auto        iter       = 2;
                    auto        count      = 0;
                    auto        prime_sum  = 0;
                    while (count < test_size)
                    {
                        if (is_prime (iter))
                        {
                            prime_sum += iter;
                            ++count;
                        }
                        ++iter;
                    }
                }
            );

        auto result = execute_testruns (
                test_repeat
            ,   [&] ()
                {
                    auto prime_sum =
                            range (2, INT_MAX)
                        >>  where (is_prime)
                        >>  take (test_size)
                        >>  sum ()
                        ;

                    // To suppress not used warnings
                    prime_sum = prime_sum;
                }                 
            );

        auto ratio_limit    = 1.25; 
        auto ratio          = ((double)expected)/result;
        TEST_ASSERT (true, (ratio > 1/ratio_limit && ratio < ratio_limit));
        printf (
                "Performance numbers for computing primes, expected:%d, result:%d, ratio_limit:%f, ratio:%f\r\n"
            ,   (int)expected
            ,   (int)result
            ,   ratio_limit
            ,   ratio
            );
    }

    bool run_all_tests (bool run_perfomance_tests)
    {
        // -------------------------------------------------------------------------
        test_opt        ();
        test_from       ();
        test_range      ();
        test_count      ();
        test_first      ();
        test_sum        ();
        test_max        ();
        test_min        ();
        test_concatenate();
        test_for_each   ();
        test_to_vector  ();
        test_to_map     ();
        test_container  ();
        test_where      ();
        test_select     ();
        test_select_many();
        test_orderby    ();
        test_take       ();
        test_skip       ();
        // -------------------------------------------------------------------------
        if (run_perfomance_tests)
        {
            test_performance_sum ();
            test_performance_is_prime ();
        }
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
        return errors > 0;
        // -------------------------------------------------------------------------
    }
} 



int main ()
{
    //for (auto iter = 0; iter < 100; ++iter)
    //{
    //    run_all_tests (false);
    //}

    auto r = run_all_tests (
#if _DEBUG
            false
#else
            true
#endif
        );

    //_CrtDumpMemoryLeaks ();

    return r ? 101 : 0;
}
// ---------------------------------------------------------------------------- -

