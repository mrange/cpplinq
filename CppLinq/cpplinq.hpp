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

// ----------------------------------------------------------------------------
#pragma once
// ----------------------------------------------------------------------------
#include <algorithm>
#include <cassert>
#include <iterator>
#include <map>
#include <numeric>
#include <string>
#include <type_traits>
#include <vector>
// ----------------------------------------------------------------------------
#ifdef _MSC_VER 
#   pragma warning (push)
#       pragma warning (disable:4512)
#endif
// ----------------------------------------------------------------------------

#define CPPLINQ_METHOD 
#define CPPLINQ_INLINEMETHOD inline

// TODO:    Struggled with getting slice protection 
//          and assignment operator detection for MINGW
//          Look into using delete standard functions
// #define CPPLINQ_DETECT_INVALID_METHODS

// ----------------------------------------------------------------------------
namespace cpplinq
{               
    // -------------------------------------------------------------------------

    typedef std::size_t size_type;

    // -------------------------------------------------------------------------

    size_type const small_pod_size = sizeof (double);

    // -------------------------------------------------------------------------

    // -------------------------------------------------------------------------
    // Tedious implementation details of cpplinq
    // -------------------------------------------------------------------------

    namespace detail
    {

        // -------------------------------------------------------------------------

        size_type const invalid_size = static_cast<size_type>(-1);

        // -------------------------------------------------------------------------

        template<typename TValue>
        struct is_small_pod
        {
            enum
            {
                value = (std::is_pod<TValue>::value && (sizeof (TValue) <= small_pod_size))
            };
        };

        template<bool b, typename TIf, typename TElse>
        struct if_else
        {
            typedef TIf type;
        };

        template<typename TIf, typename TElse>
        struct if_else<false, TIf, TElse>
        {
            typedef TElse type;
        };

        template<typename TValue>
        struct cleanup_type
        {
            typedef typename std::remove_const<
                typename std::remove_reference<TValue>::type
                >::type                                         type;
        };

        template<typename TRangeBuilder, typename TRange>
        struct get_builtup_type
        {
            static TRangeBuilder get_builder ();
            static TRange get_range ();

            typedef decltype (get_builder ().build (get_range ()))  type;
        };

        template<typename TPredicate, typename TValue>
        struct get_transformed_type
        {
            static TValue                       get_value ();
            static TPredicate                   get_predicate ();

            typedef     decltype (get_predicate ()(get_value ()))   raw_type    ;
            typedef     typename cleanup_type<raw_type>::type       type        ;
        };

        template<typename TArray>
        struct get_array_properties;

        template<typename TValue, int Size>
        struct get_array_properties<TValue[Size]>
        {
            enum
            {
                size = Size,
            };

            typedef typename    cleanup_type<TValue>::type  value_type      ;
            typedef             value_type*                 iterator_type   ;
        };

        template<typename TValue>
        struct opt
        {
            typedef     TValue  value_type;

            CPPLINQ_INLINEMETHOD opt () throw ()
                :   is_initialized (false)
            {
            }

            CPPLINQ_INLINEMETHOD explicit opt (value_type value)
                :   is_initialized      (true)
            {
                new (&storage) value_type (std::move (value));
            }

            CPPLINQ_INLINEMETHOD ~opt () throw ()
            {
                auto ptr = get_ptr ();
                if (ptr)
                {
                    ptr->~value_type ();
                }
                is_initialized = false;
            }

            CPPLINQ_INLINEMETHOD opt (opt const & v)
                :   is_initialized      (v.is_initialized)
            {
                if (v.is_initialized)
                {
                    copy (&storage  , &v.storage    );
                }
            }

            CPPLINQ_INLINEMETHOD opt (opt && v)  throw ()
                :   is_initialized      (v.is_initialized)
            {
                if (v.is_initialized)
                {
                    move (&storage  , &v.storage    );
                }
                v.is_initialized = false;
            }

            CPPLINQ_METHOD void swap (opt & v)
            {
                if (is_initialized && v.is_initialized)
                {
                    storage_type tmp;

                    move (&tmp          , &storage      );
                    move (&storage      , &v.storage    );
                    move (&v.storage    , &tmp          );
                }
                else if (is_initialized)
                {
                    move (&v.storage    , &storage      );
                    v.is_initialized= true;
                    is_initialized  = false;
                }
                else if (v.is_initialized)
                {
                    move (&storage      , &v.storage    );
                    v.is_initialized= false;
                    is_initialized  = true;
                }
                else
                {
                    // Do nothing
                }
            }

            CPPLINQ_INLINEMETHOD opt & operator= (opt const & v)
            {
                if (this == std::addressof (v))
                {
                    return *this;
                }

                opt<value_type> o (v);

                swap (o);

                return *this;
            }

            CPPLINQ_INLINEMETHOD opt & operator= (opt && v)
            {
                if (this == std::addressof (v))
                {
                    return *this;
                }

                swap (v);

                return *this;
            }

            CPPLINQ_INLINEMETHOD opt & operator= (value_type v)
            {
                return *this = opt (std::move (v));
            }

            CPPLINQ_INLINEMETHOD value_type const * get_ptr () const throw ()
            {
                if (is_initialized)
                {
                    return reinterpret_cast<value_type const *> (&storage);
                }
                else
                {
                    return nullptr;
                }
            }

            CPPLINQ_INLINEMETHOD value_type * get_ptr () throw ()
            {
                if (is_initialized)
                {
                    return reinterpret_cast<value_type*> (&storage);
                }
                else
                {
                    return nullptr;
                }
            }

            CPPLINQ_INLINEMETHOD value_type const & get () const throw ()
            {
                assert (is_initialized);
                return *get_ptr ();
            }

            CPPLINQ_INLINEMETHOD value_type & get () throw ()
            {
                assert (is_initialized);
                return *get_ptr ();
            }

            CPPLINQ_INLINEMETHOD bool has_value () const throw ()
            {
                return is_initialized;
            }

            typedef bool (opt::*type_safe_bool_type) () const;

            CPPLINQ_INLINEMETHOD operator type_safe_bool_type () const throw ()
            {
                return is_initialized ? &opt::has_value : nullptr;
            }

            CPPLINQ_INLINEMETHOD value_type const & operator* () const throw ()
            {
                return get ();
            }

            CPPLINQ_INLINEMETHOD value_type & operator* () throw ()
            {
                return get ();
            }

            CPPLINQ_INLINEMETHOD value_type const * operator-> () const throw ()
            {
                return get_ptr ();
            }

            CPPLINQ_INLINEMETHOD value_type * operator-> () throw ()
            {
                return get_ptr ();
            }

        private:
            typedef typename std::aligned_storage<
                    sizeof (value_type)
                ,   std::alignment_of<value_type>::value
                >::type storage_type    ;

            storage_type    storage         ;
            bool            is_initialized  ;

            CPPLINQ_INLINEMETHOD static void move (
                    storage_type * to
                ,   storage_type * from
                ) throw ()
            {
                auto f = reinterpret_cast<value_type*> (from); 
                new (to) value_type (std::move (*f));
                f->~value_type ();
            }

            CPPLINQ_INLINEMETHOD static void copy (
                    storage_type * to
                ,   storage_type const * from
                )
            {
                auto f = reinterpret_cast<value_type const *> (from); 
                new (to) value_type (*f);
            }


        };

        // -------------------------------------------------------------------------
        // The generic interface
        // -------------------------------------------------------------------------
        // _range classes:
        //      inherit base_range
        //      COPYABLE
        //      MOVEABLE (movesemantics)
        //      typedef                 ...         this_type       ;
        //      typedef                 ...         value_type      ;
        //      typedef                 ...         return_type     ;   // value_type | value_type const &
        //      enum { returns_reference = 0|1 };
        //      return_type front () const
        //      bool next ()
        //      template<typename TRangeBuilder>
        //      typename get_builtup_type<TRangeBuilder, this_type>::type operator>>(TRangeBuilder range_builder) const 
        // -------------------------------------------------------------------------
        // _builder classes:
        //      inherit base_builder
        //      COPYABLE
        //      MOVEABLE (movesemantics)
        //      typedef                 ...         this_type       ;
        //      template<typename TRange>
        //      TAggregated build (TRange range) const
        // -------------------------------------------------------------------------

        struct base_range 
        {
#ifdef CPPLINQ_DETECT_INVALID_METHODS
        protected:
            // In order to prevent object slicing

            CPPLINQ_INLINEMETHOD base_range () throw ()
            {
            }

            CPPLINQ_INLINEMETHOD base_range (base_range const &) throw ()
            {
            }

            CPPLINQ_INLINEMETHOD base_range (base_range &&) throw ()
            {
            }

            CPPLINQ_INLINEMETHOD ~base_range () throw ()
            {
            }

        private:
            CPPLINQ_INLINEMETHOD base_range & operator= (base_range const &);
            CPPLINQ_INLINEMETHOD base_range & operator= (base_range &&);
#endif
        };

        struct base_builder
        {
#ifdef CPPLINQ_DETECT_INVALID_METHODS
        protected:
            // In order to prevent object slicing

            CPPLINQ_INLINEMETHOD base_builder () throw ()
            {
            }

            CPPLINQ_INLINEMETHOD base_builder (base_builder const &) throw ()
            {
            }

            CPPLINQ_INLINEMETHOD base_builder (base_builder &&) throw ()
            {
            }

            CPPLINQ_INLINEMETHOD ~base_builder () throw ()
            {
            }

        private:
            CPPLINQ_INLINEMETHOD base_builder & operator= (base_builder const &);
            CPPLINQ_INLINEMETHOD base_builder & operator= (base_builder &&);
#endif
        };


        template<typename TValueIterator>
        struct from_range : base_range
        {
            static TValueIterator get_iterator ();

            typedef                 from_range<TValueIterator>          this_type       ;
            typedef                 TValueIterator                      iterator_type   ;

            typedef                 decltype (*get_iterator ())         raw_value_type  ;
            typedef        typename cleanup_type<raw_value_type>::type  value_type      ;
            enum    
            { 
                returns_reference = is_small_pod<value_type>::value ? 0 : 1 , 
            };
            typedef        typename if_else<
                                            returns_reference
                                        ,   value_type const &
                                        ,   value_type>::type           return_type     ;

            iterator_type           begin   ;
            iterator_type           end     ;

            bool                    start   ;
            iterator_type           current ;


            CPPLINQ_INLINEMETHOD from_range (
                    iterator_type begin
                ,   iterator_type end
                ) throw ()
                :   begin   (std::move (begin))
                ,   end     (std::move (end))
                ,   start   (true)
            {
            }

            CPPLINQ_INLINEMETHOD from_range (from_range const & v) throw ()
                :   begin   (v.begin)
                ,   end     (v.end)
                ,   start   (v.start)
                ,   current (v.current)
            {
            }
        
            CPPLINQ_INLINEMETHOD from_range (from_range && v) throw ()
                :   begin   (std::move (v.begin))
                ,   end     (std::move (v.end))
                ,   start   (std::move (v.start))
                ,   current (std::move (v.current))
            {
            }
        
            template<typename TRangeBuilder>
            CPPLINQ_INLINEMETHOD typename get_builtup_type<TRangeBuilder, this_type>::type operator>>(TRangeBuilder range_builder) const throw ()   
            {
                return range_builder.build (*this);
            }

            CPPLINQ_INLINEMETHOD return_type front () const 
            {
                assert (!start);
                assert (current != end);

                return *current;
            }

            CPPLINQ_INLINEMETHOD bool next () throw ()
            {
                if (start)
                {
                    start = false;
                    current = begin;
                }
                else if (current != end)
                {
                    ++current;
                }

                return current != end;
            }
        };

        // -------------------------------------------------------------------------

        // -------------------------------------------------------------------------

        template<typename TContainer>
        struct from_copy_range : base_range
        {
            typedef                 from_copy_range<TContainer>         this_type       ;

            typedef                 TContainer                          container_type  ;
            typedef        typename TContainer::const_iterator          iterator_type   ;
            typedef        typename TContainer::value_type              value_type      ;
            enum    
            { 
                returns_reference = is_small_pod<value_type>::value ? 0 : 1 , 
            };
            typedef        typename if_else<
                                            returns_reference
                                        ,   value_type const &
                                        ,   value_type>::type           return_type     ;

            container_type          container   ;

            bool                    start       ;
            iterator_type           current     ;

            CPPLINQ_INLINEMETHOD from_copy_range (
                    container_type  container
                )
                :   container   (std::move (container))
                ,   start       (true)
            {
            }

            CPPLINQ_INLINEMETHOD from_copy_range (from_copy_range const & v)
                :   container   (v.container)
                ,   start       (v.start)
            {
            }
        
            CPPLINQ_INLINEMETHOD from_copy_range (from_copy_range && v) throw ()
                :   container   (std::move (v.container))
                ,   start       (v.start)
            {
            }
        
            template<typename TRangeBuilder>
            CPPLINQ_INLINEMETHOD typename get_builtup_type<TRangeBuilder, this_type>::type operator>>(TRangeBuilder range_builder) const throw ()   
            {
                return range_builder.build (*this);
            }

            CPPLINQ_INLINEMETHOD return_type front () const 
            {
                assert (!start);
                assert (current != container.end ());

                return *current;
            }

            CPPLINQ_INLINEMETHOD bool next () throw ()
            {
                auto end = container.end ();
                if (start)
                {
                    start = false;
                    current = container.begin ();
                }
                else if (current != end)
                {
                    ++current;
                }

                return current != end;
            }
        };

        // -------------------------------------------------------------------------

        struct int_range : base_range
        {
            typedef                 int_range                           this_type       ;
            typedef                 int                                 value_type      ;
            typedef                 int                                 return_type     ;
            enum    
            { 
                returns_reference = 0   , 
            };

            bool                    start   ;
            int                     current ;
            int                     end     ;

            static int get_current (int begin, int end)
            {
                return begin < end ? begin : end;
            }

            static int get_end (int begin, int end)
            {
                return begin < end ? end : begin;
            }

            CPPLINQ_INLINEMETHOD int_range (
                    int begin
                ,   int end
                ) throw ()
                :   start   (true)
                ,   current (get_current (begin, end))
                ,   end     (get_end (begin,end))
            {
            }

            CPPLINQ_INLINEMETHOD int_range (int_range const & v) throw ()
                :   start   (v.start)
                ,   current (v.current)
                ,   end     (v.end)
            {
            }
        
            CPPLINQ_INLINEMETHOD int_range (int_range && v) throw ()
                :   start   (std::move (v.start))
                ,   current (std::move (v.current))
                ,   end     (std::move (v.end))
            {
            }
        
            template<typename TRangeBuilder>
            CPPLINQ_INLINEMETHOD typename get_builtup_type<TRangeBuilder, this_type>::type operator>>(TRangeBuilder range_builder) const throw ()   
            {
                return range_builder.build (*this);
            }

            CPPLINQ_INLINEMETHOD return_type front () const 
            {
                return current;
            }

            CPPLINQ_INLINEMETHOD bool next () throw ()
            {
                if (start)
                {
                    start = false;
                }
                else if (current < end)
                {
                    ++current;
                }

                return current < end;
            }
        };

        // -------------------------------------------------------------------------

        struct sorting_range : base_range
        {
#ifdef CPPLINQ_DETECT_INVALID_METHODS
        protected:
            // In order to prevent object slicing

            CPPLINQ_INLINEMETHOD sorting_range () throw ()
            {
            }

            CPPLINQ_INLINEMETHOD sorting_range (sorting_range const &) throw ()
            {
            }

            CPPLINQ_INLINEMETHOD sorting_range (sorting_range &&) throw ()
            {
            }

            CPPLINQ_INLINEMETHOD ~sorting_range () throw ()
            {
            }
        private:
            CPPLINQ_INLINEMETHOD sorting_range & operator= (sorting_range const &);
            CPPLINQ_INLINEMETHOD sorting_range & operator= (sorting_range &&);
#endif
        };

        template<typename TRange, typename TPredicate>
        struct orderby_range : sorting_range
        {

            typedef                 orderby_range<TRange, TPredicate>   this_type               ;
            typedef                 TRange                              range_type              ;
            typedef                 TPredicate                          predicate_type          ;

            typedef                 typename TRange::value_type         value_type              ;
            typedef                 typename TRange::return_type        forwarding_return_type  ;
            typedef                 value_type const &                  return_type             ;
            enum    
            { 
                forward_returns_reference   = TRange::returns_reference ,
                returns_reference           = 1                         , 
            };

            range_type              range           ;
            predicate_type          predicate       ;
            bool                    sort_ascending  ;

            size_type               current         ;
            std::vector<value_type> sorted_values   ;    

            CPPLINQ_INLINEMETHOD orderby_range (
                    range_type      range
                ,   predicate_type  predicate
                ,   bool            sort_ascending
                ) throw ()
                :   range           (std::move (range))
                ,   predicate       (std::move (predicate))
                ,   sort_ascending  (sort_ascending)
                ,   current         (invalid_size)
            {
                static_assert (
                        !std::is_convertible<range_type, sorting_range>::value
                    ,   "orderby may not follow orderby or thenby"
                    );
            }

            CPPLINQ_INLINEMETHOD orderby_range (orderby_range const & v)
                :   range           (v.range)
                ,   predicate       (v.predicate)
                ,   sort_ascending  (v.sort_ascending)
                ,   current         (v.current)
                ,   sorted_values   (v.sorted_values)
            {
            }

            CPPLINQ_INLINEMETHOD orderby_range (orderby_range && v) throw ()
                :   range           (std::move (v.range))
                ,   predicate       (std::move (v.predicate))
                ,   sort_ascending  (std::move (v.sort_ascending))
                ,   current         (std::move (v.current))
                ,   sorted_values   (std::move (v.sorted_values))
            {
            }

            CPPLINQ_INLINEMETHOD forwarding_return_type forwarding_front () const 
            {
                return range.front ();
            }

            CPPLINQ_INLINEMETHOD bool forwarding_next ()
            {
                return range.next ();
            }

            CPPLINQ_INLINEMETHOD bool compare_values (value_type const & l, value_type const & r) const
            {
                if (sort_ascending)
                {
                    return predicate (l) < predicate (r);
                }
                else
                {
                    return predicate (r) < predicate (l);
                }
            }

            template<typename TRangeBuilder>
            CPPLINQ_INLINEMETHOD typename get_builtup_type<TRangeBuilder, this_type>::type operator>>(TRangeBuilder range_builder) throw ()   
            {
                return range_builder.build (*this);
            }

            CPPLINQ_INLINEMETHOD return_type front () const 
            {
                return sorted_values[current];
            }

            CPPLINQ_METHOD bool next ()
            {
                if (current == invalid_size)
                {
                    sorted_values.clear ();

                    while (range.next ())
                    {
                        sorted_values.push_back (range.front ());
                    }

                    if (sorted_values.size () == 0)
                    {
                        return false;
                    }

                    std::sort (
                            sorted_values.begin ()
                        ,   sorted_values.end ()
                        ,   [&] (value_type const & l, value_type const & r)
                            {
                                return this->compare_values (l,r);
                            }
                        );

                    current = 0U;
                    return true;
                }

                if (current < sorted_values.size ())
                {
                    ++current;
                }

                return current < sorted_values.size ();
            }
        };

        template<typename TPredicate>
        struct orderby_builder : base_builder
        {
            typedef                 orderby_builder<TPredicate> this_type       ;
            typedef                 TPredicate                  predicate_type  ;

            predicate_type          predicate       ;
            bool                    sort_ascending  ;

            CPPLINQ_INLINEMETHOD explicit orderby_builder (predicate_type predicate, bool sort_ascending) throw ()
                :   predicate       (std::move (predicate))
                ,   sort_ascending  (sort_ascending)
            {
            }

            CPPLINQ_INLINEMETHOD orderby_builder (orderby_builder const & v)
                :   predicate       (v.predicate)
                ,   sort_ascending  (sort_ascending)
            {
            }

            CPPLINQ_INLINEMETHOD orderby_builder (orderby_builder && v) throw ()
                :   predicate       (std::move (v.predicate))
                ,   sort_ascending  (std::move (sort_ascending))
            {
            }

            template<typename TRange>
            CPPLINQ_INLINEMETHOD orderby_range<TRange, TPredicate> build (TRange range) const throw ()
            {
                return orderby_range<TRange, TPredicate>(range, predicate, sort_ascending);
            }

        };

        // -------------------------------------------------------------------------

        template<typename TRange, typename TPredicate>
        struct thenby_range : sorting_range
        {
            typedef                 thenby_range<TRange, TPredicate>        this_type               ;
            typedef                 TRange                                  range_type              ;
            typedef                 TPredicate                              predicate_type          ;

            typedef                 typename TRange::value_type             value_type              ;
            typedef                 typename TRange::forwarding_return_type forwarding_return_type  ;
            typedef                 value_type const &                      return_type             ;
            enum    
            { 
                forward_returns_reference   = TRange::forward_returns_reference ,
                returns_reference           = 1                                 , 
            };

            range_type              range           ;
            predicate_type          predicate       ;
            bool                    sort_ascending  ;

            size_type               current         ;
            std::vector<value_type> sorted_values   ;    

            CPPLINQ_INLINEMETHOD thenby_range (
                    range_type      range
                ,   predicate_type  predicate
                ,   bool            sort_ascending
                ) throw ()
                :   range           (std::move (range))
                ,   predicate       (std::move (predicate))
                ,   sort_ascending  (sort_ascending)
                ,   current         (invalid_size)
            {
                static_assert (
                        std::is_convertible<range_type, sorting_range>::value
                    ,   "thenby may only follow orderby or thenby"
                    );
            }

            CPPLINQ_INLINEMETHOD thenby_range (thenby_range const & v)
                :   range           (v.range)
                ,   predicate       (v.predicate)
                ,   sort_ascending  (v.sort_ascending)
                ,   current         (v.current)
                ,   sorted_values   (v.sorted_values)
            {
            }

            CPPLINQ_INLINEMETHOD thenby_range (thenby_range && v) throw ()
                :   range           (std::move (v.range))
                ,   predicate       (std::move (v.predicate))
                ,   sort_ascending  (std::move (v.sort_ascending))
                ,   current         (std::move (v.current))
                ,   sorted_values   (std::move (v.sorted_values))
            {
            }

            template<typename TRangeBuilder>
            CPPLINQ_INLINEMETHOD typename get_builtup_type<TRangeBuilder, this_type>::type operator>>(TRangeBuilder range_builder) throw ()   
            {
                return range_builder.build (*this);
            }

            CPPLINQ_INLINEMETHOD forwarding_return_type forwarding_front () const 
            {
                return range.front ();
            }

            CPPLINQ_INLINEMETHOD bool forwarding_next ()
            {
                return range.next ();
            }

            CPPLINQ_INLINEMETHOD bool compare_values (value_type const & l, value_type const & r) const
            {
                auto pless = range.compare_values (l,r);
                if (pless)
                {
                    return true;
                }

                auto pgreater = range.compare_values (r,l);
                if (pgreater)
                {
                    return false;
                }

                if (sort_ascending)
                {
                    return predicate (l) < predicate (r);
                }
                else
                {
                    return predicate (r) < predicate (l);
                }
            }

            CPPLINQ_INLINEMETHOD return_type front () const 
            {
                return sorted_values[current];
            }

            CPPLINQ_METHOD bool next ()
            {
                if (current == invalid_size)
                {
                    sorted_values.clear ();

                    while (range.forwarding_next ())
                    {
                        sorted_values.push_back (range.forwarding_front ());
                    }

                    if (sorted_values.size () == 0)
                    {
                        return false;
                    }

                    std::sort (
                            sorted_values.begin ()
                        ,   sorted_values.end ()
                        ,   [&] (value_type const & l, value_type const & r)
                            {
                                return this->compare_values (l,r);
                            }
                        );

                    current = 0U;
                    return true;
                }

                if (current < sorted_values.size ())
                {
                    ++current;
                }

                return current < sorted_values.size ();
            }
        };

        template<typename TPredicate>
        struct thenby_builder : base_builder
        {
            typedef                 thenby_builder<TPredicate>  this_type       ;
            typedef                 TPredicate                  predicate_type  ;

            predicate_type          predicate       ;
            bool                    sort_ascending  ;

            CPPLINQ_INLINEMETHOD explicit thenby_builder (predicate_type predicate, bool sort_ascending) throw ()
                :   predicate       (std::move (predicate))
                ,   sort_ascending  (sort_ascending)
            {
            }

            CPPLINQ_INLINEMETHOD thenby_builder (thenby_builder const & v)
                :   predicate       (v.predicate)
                ,   sort_ascending  (sort_ascending)
            {
            }

            CPPLINQ_INLINEMETHOD thenby_builder (thenby_builder && v) throw ()
                :   predicate       (std::move (v.predicate))
                ,   sort_ascending  (std::move (sort_ascending))
            {
            }

            template<typename TRange>
            CPPLINQ_INLINEMETHOD thenby_range<TRange, TPredicate> build (TRange range) const throw ()
            {
                return thenby_range<TRange, TPredicate>(range, predicate, sort_ascending);
            }

        };

        // -------------------------------------------------------------------------

        template<typename TRange, typename TPredicate>
        struct where_range : base_range
        {
            typedef                 where_range<TRange, TPredicate> this_type       ;
            typedef                 TRange                          range_type      ;
            typedef                 TPredicate                      predicate_type  ;

            typedef                 typename TRange::value_type     value_type      ;
            typedef                 typename TRange::return_type    return_type     ;
            enum    
            { 
                returns_reference   = TRange::returns_reference   , 
            };

            range_type              range       ;
            predicate_type          predicate   ;

            CPPLINQ_INLINEMETHOD where_range (
                    range_type      range
                ,   predicate_type  predicate
                ) throw ()
                :   range       (std::move (range))
                ,   predicate   (std::move (predicate))
            {
            }

            CPPLINQ_INLINEMETHOD where_range (where_range const & v)
                :   range       (v.range)
                ,   predicate   (v.predicate)
            {
            }

            CPPLINQ_INLINEMETHOD where_range (where_range && v) throw ()
                :   range       (std::move (v.range))
                ,   predicate   (std::move (v.predicate))
            {
            }

            template<typename TRangeBuilder>
            CPPLINQ_INLINEMETHOD typename get_builtup_type<TRangeBuilder, this_type>::type operator>>(TRangeBuilder range_builder) throw ()   
            {
                return range_builder.build (*this);
            }

            CPPLINQ_INLINEMETHOD return_type front () const 
            {
                return range.front ();
            }

            CPPLINQ_INLINEMETHOD bool next ()
            {
                while (range.next ())
                {
                    if (predicate (range.front ()))
                    {
                        return true;
                    }
                }

                return false;
            }
        };

        template<typename TPredicate>
        struct where_builder : base_builder
        {
            typedef                 where_builder<TPredicate>   this_type       ;
            typedef                 TPredicate                  predicate_type  ;

            predicate_type          predicate   ;

            CPPLINQ_INLINEMETHOD explicit where_builder (predicate_type predicate) throw ()
                :   predicate (std::move (predicate))
            {
            }

            CPPLINQ_INLINEMETHOD where_builder (where_builder const & v)
                :   predicate (v.predicate)
            {
            }

            CPPLINQ_INLINEMETHOD where_builder (where_builder && v) throw ()
                :   predicate (std::move (v.predicate))
            {
            }

            template<typename TRange>
            CPPLINQ_INLINEMETHOD where_range<TRange, TPredicate> build (TRange range) const throw ()
            {
                return where_range<TRange, TPredicate>(range, predicate);
            }

        };

        // -------------------------------------------------------------------------

        // -------------------------------------------------------------------------

        template<typename TRange>
        struct take_range : base_range
        {
            typedef                 take_range<TRange>              this_type       ;
            typedef                 TRange                          range_type      ;

            typedef                 typename TRange::value_type     value_type      ;
            typedef                 typename TRange::return_type    return_type     ;
            enum    
            { 
                returns_reference   = TRange::returns_reference   , 
            };

            range_type              range       ;
            size_type               count       ;
            size_type               current     ;


            CPPLINQ_INLINEMETHOD take_range (
                    range_type      range
                ,   size_type       count
                ) throw ()
                :   range       (std::move (range))
                ,   count       (std::move (count))
                ,   current     (0)         
            {
            }

            CPPLINQ_INLINEMETHOD take_range (take_range const & v)
                :   range       (v.range)
                ,   count       (v.count)
                ,   current     (v.current)
            {
            }

            CPPLINQ_INLINEMETHOD take_range (take_range && v) throw ()
                :   range       (std::move (v.range))
                ,   count       (std::move (v.count))
                ,   current     (std::move (v.current))
            {
            }

            template<typename TRangeBuilder>
            CPPLINQ_INLINEMETHOD typename get_builtup_type<TRangeBuilder, this_type>::type operator>>(TRangeBuilder range_builder) const throw ()   
            {
                return range_builder.build (*this);
            }

            CPPLINQ_INLINEMETHOD return_type front () const 
            {
                return range.front ();
            }

            CPPLINQ_INLINEMETHOD bool next ()
            {
                if (current < count)
                {
                    ++current;
                    return range.next ();
                }

                return false;
            }
        };

        struct take_builder : base_builder
        {
            typedef                 take_builder        this_type       ;

            size_type               count       ;

            CPPLINQ_INLINEMETHOD explicit take_builder (size_type count) throw ()
                :   count (std::move (count))
            {
            }

            CPPLINQ_INLINEMETHOD take_builder (take_builder const & v) throw ()
                :   count (v.count)
            {
            }

            CPPLINQ_INLINEMETHOD take_builder (take_builder && v) throw ()
                :   count (std::move (v.count))
            {
            }

            template<typename TRange>
            CPPLINQ_INLINEMETHOD take_range<TRange> build (TRange range) const throw ()
            {
                return take_range<TRange>(range, count);
            }

        };

        // -------------------------------------------------------------------------

        template<typename TRange, typename TPredicate>
        struct take_while_range : base_range
        {
            typedef                 take_while_range<TRange, TPredicate>    this_type       ;
            typedef                 TRange                                  range_type      ;
            typedef                 TPredicate                              predicate_type  ;

            typedef                 typename TRange::value_type             value_type      ;
            typedef                 typename TRange::return_type            return_type     ;
            enum    
            { 
                returns_reference   = TRange::returns_reference   , 
            };

            range_type              range       ;
            predicate_type          predicate   ;
            bool                    done        ;


            CPPLINQ_INLINEMETHOD take_while_range (
                    range_type      range
                ,   predicate_type  predicate
                ) throw ()
                :   range       (std::move (range))
                ,   predicate   (std::move (predicate))
                ,   done        (false)
            {
            }

            CPPLINQ_INLINEMETHOD take_while_range (take_while_range const & v)
                :   range       (v.range)
                ,   predicate   (v.predicate)
                ,   done        (v.done)
            {
            }

            CPPLINQ_INLINEMETHOD take_while_range (take_while_range && v) throw ()
                :   range       (std::move (v.range))
                ,   predicate   (std::move (v.predicate))
                ,   done        (std::move (v.done))
            {
            }

            template<typename TRangeBuilder>
            CPPLINQ_INLINEMETHOD typename get_builtup_type<TRangeBuilder, this_type>::type operator>>(TRangeBuilder range_builder) const throw ()   
            {
                return range_builder.build (*this);
            }

            CPPLINQ_INLINEMETHOD return_type front () const 
            {
                return range.front ();
            }

            CPPLINQ_INLINEMETHOD bool next ()
            {
                if (done)
                {
                    return false;
                }

                if (!range.next ())
                {
                    done = true;
                    return false;
                }

                if (!predicate (range.front ()))
                {
                    done = true;
                    return false;
                }

                return true;
            }
        };

        template<typename TPredicate>
        struct take_while_builder : base_builder
        {
            typedef                 take_while_builder<TPredicate>  this_type       ;
            typedef                 TPredicate                      predicate_type  ;

            predicate_type          predicate   ;

            CPPLINQ_INLINEMETHOD take_while_builder (predicate_type predicate) throw ()
                :   predicate (std::move (predicate))
            {
            }

            CPPLINQ_INLINEMETHOD take_while_builder (take_while_builder const & v) throw ()
                :   predicate (v.predicate)
            {
            }

            CPPLINQ_INLINEMETHOD take_while_builder (take_while_builder && v) throw ()
                :   predicate (std::move (v.predicate))
            {
            }

            template<typename TRange>
            CPPLINQ_INLINEMETHOD take_while_range<TRange, TPredicate> build (TRange range) const throw ()
            {
                return take_while_range<TRange, TPredicate>(range, predicate);
            }

        };

        // -------------------------------------------------------------------------

        template<typename TRange>
        struct skip_range : base_range
        {
            typedef                 skip_range<TRange>              this_type       ;
            typedef                 TRange                          range_type      ;

            typedef                 typename TRange::value_type     value_type      ;
            typedef                 typename TRange::return_type    return_type     ;
            enum    
            { 
                returns_reference   = TRange::returns_reference   , 
            };

            range_type              range       ;
            size_type               count       ;
            size_type               current     ;

            CPPLINQ_INLINEMETHOD skip_range (
                    range_type      range
                ,   size_type       count
                ) throw ()
                :   range       (std::move (range))
                ,   count       (std::move (count))
                ,   current     (0)
            {
            }

            CPPLINQ_INLINEMETHOD skip_range (skip_range const & v)
                :   range       (v.range)
                ,   count       (v.count)
                ,   current     (v.current)
            {
            }

            CPPLINQ_INLINEMETHOD skip_range (skip_range && v) throw ()
                :   range       (std::move (v.range))
                ,   count       (std::move (v.count))
                ,   current     (std::move (v.current))
            {
            }

            template<typename TRangeBuilder>
            CPPLINQ_INLINEMETHOD typename get_builtup_type<TRangeBuilder, this_type>::type operator>>(TRangeBuilder range_builder) const throw ()   
            {
                return range_builder.build (*this);
            }

            CPPLINQ_INLINEMETHOD return_type front () const 
            {
                return range.front ();
            }

            CPPLINQ_INLINEMETHOD bool next ()
            {
                if (current == invalid_size)
                {
                    return false;
                }

                while (current < count && range.next ())
                {
                    ++current;              
                }

                if (current < count)
                {
                    current = invalid_size;
                    return false;
                }

                return range.next ();
            }
        };

        struct skip_builder : base_builder
        {
            typedef                 skip_builder        this_type       ;

            size_type               count       ;

            CPPLINQ_INLINEMETHOD explicit skip_builder (size_type count) throw ()
                :   count (std::move (count))
            {
            }

            CPPLINQ_INLINEMETHOD skip_builder (skip_builder const & v) throw ()
                :   count (v.count)
            {
            }

            CPPLINQ_INLINEMETHOD skip_builder (skip_builder && v) throw ()
                :   count (std::move (v.count))
            {
            }

            template<typename TRange>
            CPPLINQ_INLINEMETHOD skip_range<TRange> build (TRange range) const throw ()
            {
                return skip_range<TRange>(range, count);
            }

        };

        // -------------------------------------------------------------------------

        template<typename TRange, typename TPredicate>
        struct skip_while_range : base_range
        {
            typedef                 skip_while_range<TRange, TPredicate>    this_type       ;
            typedef                 TRange                                  range_type      ;
            typedef                 TPredicate                              predicate_type  ;

            typedef                 typename TRange::value_type     value_type      ;
            typedef                 typename TRange::return_type    return_type     ;
            enum    
            { 
                returns_reference   = TRange::returns_reference   , 
            };

            range_type              range       ;
            predicate_type          predicate   ;
            bool                    skipping    ;

            CPPLINQ_INLINEMETHOD skip_while_range (
                    range_type      range
                ,   predicate_type  predicate
                ) throw ()
                :   range       (std::move (range))
                ,   predicate   (std::move (predicate))
                ,   skipping    (true)
            {
            }

            CPPLINQ_INLINEMETHOD skip_while_range (skip_while_range const & v)
                :   range       (v.range)
                ,   predicate   (v.predicate)
                ,   skipping    (v.skipping)
            {
            }

            CPPLINQ_INLINEMETHOD skip_while_range (skip_while_range && v) throw ()
                :   range       (std::move (v.range))
                ,   predicate   (std::move (v.predicate))
                ,   skipping    (std::move (v.skipping))
            {
            }

            template<typename TRangeBuilder>
            CPPLINQ_INLINEMETHOD typename get_builtup_type<TRangeBuilder, this_type>::type operator>>(TRangeBuilder range_builder) const throw ()   
            {
                return range_builder.build (*this);
            }

            CPPLINQ_INLINEMETHOD return_type front () const 
            {
                return range.front ();
            }

            CPPLINQ_INLINEMETHOD bool next ()
            {
                if (!skipping)
                {
                    return range.next ();
                }

                while (range.next ())
                {
                    if (!predicate (range.front ()))
                    {
                        skipping = false;
                        return true;
                    }
                }

                return false;
            }                
        };

        template <typename TPredicate>
        struct skip_while_builder : base_builder
        {
            typedef                 skip_while_builder<TPredicate>  this_type       ;
            typedef                 TPredicate                      predicate_type  ;

            predicate_type          predicate   ;

            CPPLINQ_INLINEMETHOD skip_while_builder (predicate_type predicate) throw ()
                :   predicate (std::move (predicate))
            {
            }

            CPPLINQ_INLINEMETHOD skip_while_builder (skip_while_builder const & v) throw ()
                :   predicate (v.predicate)
            {
            }

            CPPLINQ_INLINEMETHOD skip_while_builder (skip_while_builder && v) throw ()
                :   predicate (std::move (v.predicate))
            {
            }

            template<typename TRange>
            CPPLINQ_INLINEMETHOD skip_while_range<TRange, TPredicate> build (TRange range) const throw ()
            {
                return skip_while_range<TRange, TPredicate>(range, predicate);
            }

        };

        // -------------------------------------------------------------------------

        template<typename TRange, typename TPredicate>
        struct select_range : base_range
        {
            static typename TRange::value_type get_source ();
            static          TPredicate get_predicate ();


            typedef         decltype (get_predicate ()(get_source ()))  raw_value_type  ;
            typedef        typename cleanup_type<raw_value_type>::type  value_type      ;
            typedef                 value_type                          return_type     ;
            enum    
            { 
                returns_reference   = 0   , 
            };

            typedef                 select_range<TRange, TPredicate>    this_type       ;
            typedef                 TRange                              range_type      ;
            typedef                 TPredicate                          predicate_type  ;

            range_type              range       ;
            predicate_type          predicate   ;

            CPPLINQ_INLINEMETHOD select_range (
                    range_type      range
                ,   predicate_type  predicate
                ) throw ()
                :   range       (std::move (range))
                ,   predicate   (std::move (predicate))
            {
            }

            CPPLINQ_INLINEMETHOD select_range (select_range const & v)
                :   range       (v.range)
                ,   predicate   (v.predicate)
            {
            }

            CPPLINQ_INLINEMETHOD select_range (select_range && v) throw ()
                :   range       (std::move (v.range))
                ,   predicate   (std::move (v.predicate))
            {
            }

            template<typename TRangeBuilder>
            CPPLINQ_INLINEMETHOD typename get_builtup_type<TRangeBuilder, this_type>::type operator>>(TRangeBuilder range_builder) const throw ()   
            {
                return range_builder.build (*this);
            }

            CPPLINQ_INLINEMETHOD return_type front () const 
            {
                return predicate (range.front ());
            }

            CPPLINQ_INLINEMETHOD bool next ()
            {
                return range.next ();
            }
        };

        template<typename TPredicate>
        struct select_builder : base_builder
        {
            typedef                 select_builder<TPredicate>  this_type       ;
            typedef                 TPredicate                  predicate_type  ;

            predicate_type          predicate   ;

            CPPLINQ_INLINEMETHOD explicit select_builder (predicate_type predicate) throw ()
                :   predicate (std::move (predicate))
            {
            }

            CPPLINQ_INLINEMETHOD select_builder (select_builder const & v)
                :   predicate (v.predicate)
            {
            }

            CPPLINQ_INLINEMETHOD select_builder (select_builder && v) throw ()
                :   predicate (std::move (v.predicate))
            {
            }

            template<typename TRange>
            CPPLINQ_INLINEMETHOD select_range<TRange, TPredicate> build (TRange range) const throw ()
            {
                return select_range<TRange, TPredicate>(range, predicate);
            }

        };

        // -------------------------------------------------------------------------

        // Some trickery in order to force the code to compile on VS2012
        template<typename TRange, typename TPredicate>
        struct select_many_range_helper
        {
            static typename TRange::value_type get_source ();
            static          TPredicate get_predicate ();

            typedef        decltype (get_predicate ()(get_source ()))           raw_inner_range_type    ;
            typedef        typename cleanup_type<raw_inner_range_type>::type    inner_range_type        ;

            static         inner_range_type get_inner_range ();

            typedef        decltype (get_inner_range ().front ())               raw_value_type          ;
            typedef        typename cleanup_type<raw_value_type>::type          value_type              ;

        };

        template<typename TRange, typename TPredicate>
        struct select_many_range : base_range
        {
            typedef select_many_range_helper<TRange, TPredicate>    helper_type         ;

            typedef        typename helper_type::inner_range_type   inner_range_type    ;
            typedef        typename helper_type::value_type         value_type          ;
            typedef                 value_type                      return_type         ;
            enum    
            { 
                returns_reference   = 0   , 
            };

            typedef                 select_many_range<TRange, TPredicate>       this_type               ;
            typedef                 TRange                                      range_type              ;
            typedef                 TPredicate                                  predicate_type          ;

            range_type              range       ;
            predicate_type          predicate   ;

            opt<inner_range_type>   inner_range ;
            

            CPPLINQ_INLINEMETHOD select_many_range (
                    range_type      range
                ,   predicate_type  predicate
                ) throw ()
                :   range       (std::move (range))
                ,   predicate   (std::move (predicate))
            {
            }

            CPPLINQ_INLINEMETHOD select_many_range (select_many_range const & v)
                :   range       (v.range)
                ,   predicate   (v.predicate)
                ,   inner_range (v.inner_range)
            {
            }

            CPPLINQ_INLINEMETHOD select_many_range (select_many_range && v) throw ()
                :   range       (std::move (v.range))
                ,   predicate   (std::move (v.predicate))
                ,   inner_range (std::move (v.inner_range))
            {
            }

            template<typename TRangeBuilder>
            CPPLINQ_INLINEMETHOD typename get_builtup_type<TRangeBuilder, this_type>::type operator>>(TRangeBuilder range_builder) const throw ()   
            {
                return range_builder.build (*this);
            }

            CPPLINQ_INLINEMETHOD return_type front () const 
            {
                assert (inner_range);
                return inner_range->front ();
            }

            CPPLINQ_INLINEMETHOD bool next ()
            {
                if (inner_range && inner_range->next ())
                {
                    return true;
                }

                if (range.next ())
                {
                    inner_range = predicate (range.front ());
                    return inner_range && inner_range->next ();
                }

                return false;
            }
        };

        template<typename TPredicate>
        struct select_many_builder : base_builder
        {
            typedef                 select_many_builder<TPredicate> this_type       ;
            typedef                 TPredicate                      predicate_type  ;

            predicate_type          predicate   ;

            CPPLINQ_INLINEMETHOD explicit select_many_builder (predicate_type predicate) throw ()
                :   predicate (std::move (predicate))
            {
            }

            CPPLINQ_INLINEMETHOD select_many_builder (select_many_builder const & v)
                :   predicate (v.predicate)
            {
            }

            CPPLINQ_INLINEMETHOD select_many_builder (select_many_builder && v) throw ()
                :   predicate (std::move (v.predicate))
            {
            }

            template<typename TRange>
            CPPLINQ_INLINEMETHOD select_many_range<TRange, TPredicate> build (TRange range) const throw ()
            {
                return select_many_range<TRange, TPredicate>(range, predicate);
            }

        };

        namespace experimental
        {
            // -------------------------------------------------------------------------

            // TODO: Verify that container range aggregator has the right semantics

            template<typename TRange>
            struct container_iterator
            {
                typedef                 std::forward_iterator_tag   iterator_category   ;
                typedef     typename    TRange::value_type          value_type          ;
                typedef     typename    TRange::return_type         return_type         ;
                enum    
                { 
                    returns_reference   = TRange::returns_reference   , 
                };

                typedef                 std::ptrdiff_t              difference_type     ;
                typedef                 value_type*                 pointer             ;
                typedef                 value_type&                 reference           ;

                typedef                 container_iterator<TRange>  this_type   ;
                typedef                 TRange                      range_type  ;

                bool                    has_value                               ;
                opt<range_type>         range                                   ;

                CPPLINQ_INLINEMETHOD container_iterator ()   throw ()
                    :   has_value   (false)
                {
                }

                CPPLINQ_INLINEMETHOD container_iterator (range_type r)   throw ()
                    :   range      (std::move (r))
                {
                    has_value = range && range->next (); 
                }

                CPPLINQ_INLINEMETHOD container_iterator (container_iterator const & v) throw ()
                    :   has_value   (v.has_value)
                    ,   range       (v.range)
                {
                }

                CPPLINQ_INLINEMETHOD container_iterator (container_iterator && v) throw ()
                    :   has_value   (std::move (v.has_value))
                    ,   range       (std::move (v.range))
                {
                }

                CPPLINQ_INLINEMETHOD return_type        operator* () const
                {
                    assert (has_value);
                    assert (range);
                    return range->front ();
                }

                CPPLINQ_INLINEMETHOD value_type const * operator-> () const
                {
                    static_assert (
                            returns_reference
                        ,   "operator-> requires a range that returns a reference, typically select causes ranges to return values not references"
                        );
                    return &range->front ();
                }

                CPPLINQ_INLINEMETHOD this_type & operator++()
                {
                    if (has_value && range)
                    {
                        has_value = range->next ();
                    }

                    return *this;
                }

                CPPLINQ_INLINEMETHOD bool operator== (this_type const & v) const throw ()
                {
                    if (!has_value && !v.has_value)
                    {
                        return true;
                    }
                    else if (has_value && has_value && range.get_ptr () == v.range.get_ptr ())
                    {
                        return true;
                    }
                    else
                    {
                        return false;
                    }
                }

                CPPLINQ_INLINEMETHOD bool operator!= (this_type const & v) const throw ()
                {
                    return !(*this == v);
                }
            };

            template<typename TRange>
            struct container
            {
                typedef                 container<TRange>   this_type                   ;
                typedef                 TRange              range_type                  ;
                typedef                 typename TRange::value_type     value_type      ;
                typedef                 typename TRange::return_type    return_type     ;
                enum    
                { 
                    returns_reference   = TRange::returns_reference   , 
                };

                range_type              range   ;

                CPPLINQ_INLINEMETHOD explicit container (TRange range)
                    :   range (std::move (range))
                {
                }

                CPPLINQ_INLINEMETHOD container (container const & v) throw ()
                    :   range       (v.range)
                {
                }

                CPPLINQ_INLINEMETHOD container (container && v) throw ()
                    :   range       (std::move (v.range))
                {
                }

                CPPLINQ_INLINEMETHOD container_iterator<TRange>  begin () throw ()
                {
                    return container_iterator<TRange>(range);
                }

                CPPLINQ_INLINEMETHOD container_iterator<TRange>  end () throw ()
                {
                    return container_iterator<TRange>();
                }

            };

            struct container_builder : base_builder
            {
                typedef                 container_builder       this_type       ;

                CPPLINQ_INLINEMETHOD container_builder () throw ()
                {
                }

                CPPLINQ_INLINEMETHOD container_builder (container_builder const & v) throw ()
                {
                }

                CPPLINQ_INLINEMETHOD container_builder (container_builder && v) throw ()
                {
                }

                template<typename TRange>
                CPPLINQ_METHOD container<TRange> build (TRange range)
                {
                    return container<TRange> (std::move (range));
                }

            };
        }

        // -------------------------------------------------------------------------

        struct to_vector_builder : base_builder
        {
            typedef                 to_vector_builder       this_type       ;

            size_type               capacity;

            CPPLINQ_INLINEMETHOD explicit to_vector_builder (size_type capacity = 16U) throw ()
                :   capacity    (capacity)
            {
            }

            CPPLINQ_INLINEMETHOD to_vector_builder (to_vector_builder const & v) throw ()
                :   capacity (v.capacity)
            {
            }

            CPPLINQ_INLINEMETHOD to_vector_builder (to_vector_builder && v) throw ()
                :   capacity (std::move (v.capacity))
            {
            }

            template<typename TRange>
            CPPLINQ_METHOD std::vector<typename TRange::value_type> build (TRange range)
            {
                std::vector<typename TRange::value_type> result;
                result.reserve (capacity);

                while (range.next ())
                {
                    result.push_back (range.front ());
                }

                return std::move (result);
            }

        };

        // -------------------------------------------------------------------------

        template<typename TKeyPredicate>
        struct to_map_builder : base_builder
        {
            static TKeyPredicate get_key_predicate ();

            typedef                     to_map_builder<TKeyPredicate>  this_type           ;
            typedef                     TKeyPredicate               key_predicate_type  ;

            key_predicate_type          key_predicate   ;

            CPPLINQ_INLINEMETHOD explicit to_map_builder (key_predicate_type key_predicate) throw ()
                :   key_predicate   (std::move (key_predicate))
            {
            }

            CPPLINQ_INLINEMETHOD to_map_builder (to_map_builder const & v)
                :   key_predicate (v.key_predicate)
            {
            }

            CPPLINQ_INLINEMETHOD to_map_builder (to_map_builder && v) throw ()
                :   key_predicate (std::move (v.key_predicate))
            {
            }

            template<typename TRange>
            CPPLINQ_METHOD std::map<
                    typename get_transformed_type<key_predicate_type, typename TRange::value_type>::type
                ,   typename TRange::value_type
                > build (TRange range)
            {
                typedef std::map<
                    typename get_transformed_type<key_predicate_type, typename TRange::value_type>::type
                ,   typename TRange::value_type
                >   result_type;

                result_type result;

                while (range.next ())
                {
                    auto v = range.front ();
                    auto k = key_predicate (v);
                    
                    result.insert (typename result_type::value_type (std::move (k), std::move (v)));
                }

                return std::move (result);
            }

        };

        // -------------------------------------------------------------------------

        template<typename TPredicate>
        struct for_each_builder : base_builder
        {
            typedef                 for_each_builder<TPredicate>    this_type       ;
            typedef                 TPredicate                      predicate_type  ;

            predicate_type          predicate;

            CPPLINQ_INLINEMETHOD explicit for_each_builder (predicate_type predicate) throw ()
                :   predicate    (std::move (predicate))
            {
            }

            CPPLINQ_INLINEMETHOD for_each_builder (for_each_builder const & v) throw ()
                :   predicate (v.predicate)
            {
            }

            CPPLINQ_INLINEMETHOD for_each_builder (for_each_builder && v) throw ()
                :   predicate (std::move (v.predicate))
            {
            }


            template<typename TRange>
            CPPLINQ_INLINEMETHOD void build (TRange range)
            {
                while (range.next ())
                {
                    predicate (range.front ());
                }
            }

        };

        // -------------------------------------------------------------------------

        struct first_builder : base_builder
        {
            typedef                 first_builder                   this_type       ;

            CPPLINQ_INLINEMETHOD first_builder () throw ()
            {
            }

            CPPLINQ_INLINEMETHOD first_builder (first_builder const & v) throw ()
            {
            }           

            CPPLINQ_INLINEMETHOD first_builder (first_builder && v) throw ()
            {
            }

            template<typename TRange>
            CPPLINQ_INLINEMETHOD typename TRange::value_type build (TRange range)
            {
                if (range.next ())
                {
                    return range.front ();
                }

                return typename TRange::value_type ();
            }

        };

        // -------------------------------------------------------------------------

        struct count_builder : base_builder
        {
            typedef                 count_builder                   this_type       ;

            CPPLINQ_INLINEMETHOD count_builder () throw ()
            {
            }

            CPPLINQ_INLINEMETHOD count_builder (count_builder const & v) throw ()
            {
            }

            CPPLINQ_INLINEMETHOD count_builder (count_builder && v) throw ()
            {
            }


            template<typename TRange>
            CPPLINQ_INLINEMETHOD size_type build (TRange range)
            {
                size_type count = 0U;
                while (range.next ())
                {
                    ++count;
                }
                return count;
            }

        };

        // -------------------------------------------------------------------------

        struct sum_builder : base_builder
        {
            typedef                 sum_builder                     this_type       ;

            CPPLINQ_INLINEMETHOD sum_builder () throw ()
            {
            }

            CPPLINQ_INLINEMETHOD sum_builder (sum_builder const & v) throw ()
            {
            }

            CPPLINQ_INLINEMETHOD sum_builder (sum_builder && v) throw ()
            {
            }

            template<typename TRange>
            CPPLINQ_INLINEMETHOD typename TRange::value_type build (TRange range)
            {
                auto sum = typename TRange::value_type ();
                while (range.next ())
                {
                    sum += range.front ();
                }
                return std::move (sum);
            }

        };

        // -------------------------------------------------------------------------

        struct max_builder : base_builder
        {
            typedef                 max_builder                         this_type       ;

            CPPLINQ_INLINEMETHOD max_builder () throw ()
            {
            }

            CPPLINQ_INLINEMETHOD max_builder (max_builder const & v) throw ()
            {
            }

            CPPLINQ_INLINEMETHOD max_builder (max_builder && v) throw ()
            {
            }


            template<typename TRange>
            CPPLINQ_INLINEMETHOD typename TRange::value_type build (TRange range)
            {
                auto current = std::numeric_limits<typename TRange::value_type>::min ();
                while (range.next ())
                {
                    auto v = range.front ();
                    if (current < v)
                    {
                        current = std::move (v);
                    }
                }

                return std::move (current);
            }

        };

        // -------------------------------------------------------------------------

        struct min_builder : base_builder
        {
            typedef                 min_builder                         this_type       ;

            CPPLINQ_INLINEMETHOD min_builder () throw ()
            {
            }

            CPPLINQ_INLINEMETHOD min_builder (min_builder const & v) throw ()
            {
            }

            CPPLINQ_INLINEMETHOD min_builder (min_builder && v) throw ()
            {
            }


            template<typename TRange>
            CPPLINQ_INLINEMETHOD typename TRange::value_type build (TRange range)
            {
                auto current = std::numeric_limits<typename TRange::value_type>::max ();
                while (range.next ())
                {
                    auto v = range.front ();
                    if (v < current)
                    {
                        current = std::move (v);
                    }
                }

                return std::move (current);
            }

        };

        // -------------------------------------------------------------------------

        struct avg_builder : base_builder
        {
            typedef                 avg_builder                         this_type       ;

            CPPLINQ_INLINEMETHOD avg_builder () throw ()
            {
            }

            CPPLINQ_INLINEMETHOD avg_builder (avg_builder const & v) throw ()
            {
            }

            CPPLINQ_INLINEMETHOD avg_builder (avg_builder && v) throw ()
            {
            }


            template<typename TRange>
            CPPLINQ_INLINEMETHOD typename TRange::value_type build (TRange range)
            {
                auto sum = typename TRange::value_type ();
                int  count = 0;
                while (range.next ())
                {
                    sum += range.front ();
                    ++count;
                }

                if (count == 0)
                {
                    return sum;
                }

                return std::move (sum/count);
            }

        };

        // -------------------------------------------------------------------------

        template<typename TCharType>
        struct concatenate_builder : base_builder
        {
            typedef                         concatenate_builder<TCharType>  this_type       ;

            std::basic_string<TCharType>    separator   ;
            size_type                       capacity    ;

            CPPLINQ_INLINEMETHOD concatenate_builder (
                    std::basic_string<TCharType>    separator
                ,   size_type capacity
                ) throw ()
                :   separator   (std::move (separator))
                ,   capacity    (capacity)
            {
            }

            CPPLINQ_INLINEMETHOD concatenate_builder (concatenate_builder const & v) throw ()
                :   separator   (v.separator)
                ,   capacity    (v.capacity)
            {
            }

            CPPLINQ_INLINEMETHOD concatenate_builder (concatenate_builder && v) throw ()
                :   separator   (std::move (v.separator))
                ,   capacity    (std::move (v.capacity))
            {
            }

            template<typename TRange>
            CPPLINQ_INLINEMETHOD typename std::basic_string<TCharType> build (TRange range)
            {
                auto                    first  =   true ;
                std::vector<TCharType>  buffer          ;

                buffer.reserve (capacity);


                while (range.next ())
                {
                    if (first)
                    {
                        first = false;
                    }
                    else
                    {
                        buffer.insert (
                                buffer.end ()
                            ,   separator.begin ()
                            ,   separator.end ()
                            );
                    }

                    auto v = range.front ();

                    buffer.insert (
                            buffer.end ()
                        ,   v.begin ()
                        ,   v.end ()
                        );
                }

                return std::basic_string<TCharType> (
                        buffer.begin ()
                    ,   buffer.end ()
                    );
            }

        };

        // -------------------------------------------------------------------------

        struct any_builder : base_builder
        {
            typedef                 any_builder                   this_type       ;

            CPPLINQ_INLINEMETHOD any_builder () throw ()
            {
            }

            CPPLINQ_INLINEMETHOD any_builder (any_builder const & v) throw ()
            {
            }

            CPPLINQ_INLINEMETHOD any_builder (any_builder && v) throw ()
            {
            }


            template<typename TRange>
            CPPLINQ_INLINEMETHOD bool build (TRange range)
            {
                return range.next ();
            }

        };

        // -------------------------------------------------------------------------

        template <typename TPredicate>
        struct all_predicate_builder : base_builder
        {
            typedef                 all_predicate_builder<TPredicate>   this_type       ;
            typedef                 TPredicate                          predicate_type  ;

            predicate_type          predicate   ;

            CPPLINQ_INLINEMETHOD all_predicate_builder (predicate_type  predicate) throw ()
                :   predicate   (std::move (predicate))
            {
            }

            CPPLINQ_INLINEMETHOD all_predicate_builder (all_predicate_builder const & v) throw ()
                :   predicate   (v.predicate)
            {
            }

            CPPLINQ_INLINEMETHOD all_predicate_builder (all_predicate_builder && v) throw ()
                :   predicate   (std::move (v.predicate))
            {
            }

            template<typename TRange>
            CPPLINQ_INLINEMETHOD bool build (TRange range)
            {
                while (range.next ())
                {
                    if (!predicate (range.front ()))
                    {
                        return false;
                    }
                }

                return true;
            }
        };

        // -------------------------------------------------------------------------

    }   // namespace detail

    // -------------------------------------------------------------------------
    // The interface of cpplinq
    // -------------------------------------------------------------------------

    // Range sources

    template<typename TValueIterator>
    CPPLINQ_INLINEMETHOD detail::from_range<TValueIterator> from_iterators (
            TValueIterator  begin
        ,   TValueIterator  end
        ) throw ()
    {
        return detail::from_range<TValueIterator> (std::move (begin), std::move (end));
    }

    template<typename TContainer>
    CPPLINQ_INLINEMETHOD detail::from_range<typename TContainer::const_iterator> from (
            TContainer  const & container
        )
    {
        return detail::from_range<typename TContainer::const_iterator> (
                container.begin ()
            ,   container.end ()
            );
    }

    template<typename TValueArray>
    CPPLINQ_INLINEMETHOD detail::from_range<typename detail::get_array_properties<TValueArray>::iterator_type> from_array (
            TValueArray & a 
        ) throw ()
    {
        typedef detail::get_array_properties<TValueArray>   array_properties;
        typedef typename array_properties::iterator_type iterator_type; 

        iterator_type begin = a;
        iterator_type end   = begin + array_properties::size; 

        return detail::from_range<typename array_properties::iterator_type> (
                std::move (begin)
            ,   std::move (end)
            );
    }

    template<typename TContainer>
    CPPLINQ_INLINEMETHOD detail::from_copy_range<TContainer> from_copy (
            TContainer  const & container
        )
    {
        return detail::from_copy_range<TContainer> (
                std::move (container)
            );
    }

    CPPLINQ_INLINEMETHOD detail::int_range range (
            int         start
        ,   int         count
        ) throw ()
    {
        auto c      = count > 0 ? count : 0;
        auto end    = (INT_MAX - c) > start ? (start + c) : INT_MAX;
        return detail::int_range (start, end);
    }

    // Range operators

    template<typename TPredicate>
    CPPLINQ_INLINEMETHOD detail::orderby_builder<TPredicate> orderby (
            TPredicate      predicate
        ,   bool            sort_ascending  = true
        ) throw ()
    {
        return detail::orderby_builder<TPredicate> (std::move (predicate), sort_ascending);
    }

    template<typename TPredicate>
    CPPLINQ_INLINEMETHOD detail::orderby_builder<TPredicate> orderby_ascending (
            TPredicate      predicate
        ) throw ()
    {
        return detail::orderby_builder<TPredicate> (std::move (predicate), true);
    }

    template<typename TPredicate>
    CPPLINQ_INLINEMETHOD detail::orderby_builder<TPredicate> orderby_descending (
            TPredicate      predicate
        ) throw ()
    {
        return detail::orderby_builder<TPredicate> (std::move (predicate), false);
    }

    template<typename TPredicate>
    CPPLINQ_INLINEMETHOD detail::thenby_builder<TPredicate> thenby (
            TPredicate      predicate
        ,   bool            sort_ascending  = true
        ) throw ()
    {
        return detail::thenby_builder<TPredicate> (std::move (predicate), sort_ascending);
    }

    template<typename TPredicate>
    CPPLINQ_INLINEMETHOD detail::thenby_builder<TPredicate> thenby_ascending (
            TPredicate      predicate
        ) throw ()
    {
        return detail::thenby_builder<TPredicate> (std::move (predicate), true);
    }

    template<typename TPredicate>
    CPPLINQ_INLINEMETHOD detail::thenby_builder<TPredicate> thenby_descending (
            TPredicate      predicate
        ) throw ()
    {
        return detail::thenby_builder<TPredicate> (std::move (predicate), false);
    }

    template<typename TPredicate>
    CPPLINQ_INLINEMETHOD detail::where_builder<TPredicate> where (
            TPredicate      predicate
        ) throw ()
    {
        return detail::where_builder<TPredicate> (std::move (predicate));
    }

    template<typename TPredicate>
    CPPLINQ_INLINEMETHOD detail::take_while_builder<TPredicate> take_while (
            TPredicate        predicate
        ) throw ()
    {
        return detail::take_while_builder<TPredicate> (std::move (predicate));
    }

    CPPLINQ_INLINEMETHOD detail::take_builder take (
            size_type     count
        ) throw ()
    {
        return detail::take_builder (count);
    }

    template <typename TPredicate>
    CPPLINQ_INLINEMETHOD detail::skip_while_builder<TPredicate> skip_while (
            TPredicate        predicate
        ) throw ()
    {
        return detail::skip_while_builder<TPredicate> (predicate);
    }

    CPPLINQ_INLINEMETHOD detail::skip_builder skip (
            size_type       count
        ) throw ()
    {
        return detail::skip_builder (count);
    }

    template<typename TPredicate>
    CPPLINQ_INLINEMETHOD detail::select_builder<TPredicate> select (
            TPredicate      predicate
        ) throw ()
    {
        return detail::select_builder<TPredicate> (std::move (predicate));
    }

    template<typename TPredicate>
    CPPLINQ_INLINEMETHOD detail::select_many_builder<TPredicate> select_many (
            TPredicate      predicate
        ) throw ()
    {
        return detail::select_many_builder<TPredicate> (std::move (predicate));
    }

    // Range aggregators

    namespace experimental
    {
        CPPLINQ_INLINEMETHOD detail::experimental::container_builder    container () throw ()
        {
            return detail::experimental::container_builder ();
        }
    }

    CPPLINQ_INLINEMETHOD detail::to_vector_builder    to_vector (size_type capacity = 16U) throw ()
    {
        return detail::to_vector_builder (capacity);
    }

    template<typename TKeyPredicate>
    CPPLINQ_INLINEMETHOD detail::to_map_builder<TKeyPredicate>  to_map (TKeyPredicate key_predicate) throw ()
    {
        return detail::to_map_builder<TKeyPredicate>(std::move (key_predicate));
    }

    template<typename TPredicate>
    CPPLINQ_INLINEMETHOD detail::for_each_builder<TPredicate> for_each (
            TPredicate predicate
        ) throw ()
    {
        return detail::for_each_builder<TPredicate> (std::move (predicate));
    }

    CPPLINQ_INLINEMETHOD detail::first_builder   first () throw ()
    {
        return detail::first_builder ();
    }

    CPPLINQ_INLINEMETHOD detail::count_builder   count () throw ()
    {
        return detail::count_builder ();
    }

    CPPLINQ_INLINEMETHOD detail::sum_builder  sum () throw ()
    {
        return detail::sum_builder ();
    }

    CPPLINQ_INLINEMETHOD detail::max_builder  max () throw ()
    {
        return detail::max_builder ();
    }

    CPPLINQ_INLINEMETHOD detail::min_builder  min () throw ()
    {
        return detail::min_builder ();
    }

    CPPLINQ_INLINEMETHOD detail::avg_builder  avg () throw ()
    {
        return detail::avg_builder ();
    }

    CPPLINQ_INLINEMETHOD detail::concatenate_builder<char>  concatenate (
            std::string separator
        ,   size_type capacity = 16U
        ) throw ()
    {
        return detail::concatenate_builder<char> (
                std::move (separator)
            ,   capacity
            );
    }

    CPPLINQ_INLINEMETHOD detail::concatenate_builder<wchar_t>  concatenate (
            std::wstring separator
        ,   size_type capacity = 16U
        ) throw ()
    {
        return detail::concatenate_builder<wchar_t> (
                std::move (separator)
            ,   capacity
            );
    }

    CPPLINQ_INLINEMETHOD detail::any_builder   any () throw ()
    {
        return detail::any_builder ();
    }

    template <typename TPredicate>
    CPPLINQ_INLINEMETHOD detail::all_predicate_builder<TPredicate>   all (
            TPredicate predicate
        ) throw ()
    {
        return detail::all_predicate_builder<TPredicate> (std::move (predicate));
    }

    // -------------------------------------------------------------------------

}
// ----------------------------------------------------------------------------
#ifdef _MSC_VER 
#   pragma warning (pop)
#endif
// ----------------------------------------------------------------------------
