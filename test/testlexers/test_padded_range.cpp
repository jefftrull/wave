/*=============================================================================
    Boost.Wave: A Standard compliant C++ preprocessor library
    http://www.boost.org/

    Copyright (c) 2017 Jeffrey E Trull. Distributed under the Boost
    Software License, Version 1.0. (See accompanying file
    LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

// system headers
#include <vector>
#include <list>
#include <algorithm>

// things we need from other parts of Boost
#include <boost/container/slist.hpp>
#include <boost/detail/lightweight_test.hpp>

// the class being tested
#include <boost/wave/util/padded_range.hpp>

///////////////////////////////////////////////////////////////////////////////
int 
main(int argc, char *argv[])
{
    using namespace boost::wave::util;

    // basic usage
    int test_data[] = {9,8,7,6};
    padded_range<int const *, 42, 10>
        vrng(test_data, test_data + sizeof(test_data)/sizeof(int));

    int expected_with_padding[] = {9,8,7,6,42,42,42,42,42,42,42,42,42,42};

    BOOST_TEST( (std::distance(vrng.begin(), vrng.end()) ==
                 sizeof(expected_with_padding)/sizeof(int)) &&
                std::equal(vrng.begin(), vrng.end(), expected_with_padding) );
    BOOST_TEST( std::distance(vrng.begin(), vrng.pad_begin()) == sizeof(test_data)/sizeof(int) );
    BOOST_TEST_EQ( std::distance(vrng.pad_begin(), vrng.end()), 10);

    // random access (increment by 2)
    std::vector<int> ra_access;
    typedef padded_range<int const*, 42, 10>::iterator pr_iter_t;
    for (pr_iter_t it = vrng.begin(); it != vrng.end(); it += 2) {
        ra_access.push_back(*it);
    }
    int expected_ra_access[] = {9, 7, 42, 42, 42, 42, 42};

    BOOST_TEST( (std::distance(ra_access.begin(), ra_access.end()) ==
                 sizeof(expected_ra_access)/sizeof(int)) &&
                std::equal(ra_access.begin(), ra_access.end(), expected_ra_access) );

    // random access decrementing
    std::vector<int> ra_decrement;
    for (pr_iter_t it = vrng.end()-2; it >= vrng.begin(); it -= 2) {
        ra_decrement.push_back(*it);
    }
    int expected_ra_decrement[] = {42, 42, 42, 42, 42, 7, 9};

    BOOST_TEST( (std::distance(ra_decrement.begin(), ra_decrement.end()) ==
                 sizeof(expected_ra_decrement)/sizeof(int)) &&
                std::equal(ra_decrement.begin(), ra_decrement.end(), expected_ra_decrement) );

    // test BiDirectionalIterators with std::list
    int test_list_data[] = {10,11,12,13};
    std::list<int> test_list(test_list_data, test_list_data + sizeof(test_list_data)/sizeof(int));
    padded_range<std::list<int>::const_iterator, 19, 5> lrng(test_list.begin(), test_list.end());
    int expected_list[] = {10,11,12,13,19,19,19,19,19};

    BOOST_TEST( (std::distance(lrng.begin(), lrng.end()) == sizeof(expected_list)/sizeof(int)) &&
                std::equal(lrng.begin(), lrng.end(), expected_list) );

    // check decrement
    typedef padded_range<std::list<int>::const_iterator, 19, 5>::iterator pr_list_iter_t;
    pr_list_iter_t end = lrng.end(); end--;
    std::vector<int> decrement_list_result;
    for (pr_list_iter_t it = end; it != lrng.begin(); it--) {
        decrement_list_result.push_back(*it);
    }
    int expected_decrement_list_result[] = {19,19,19,19,19,13,12,11};

    BOOST_TEST( (std::distance(decrement_list_result.begin(), decrement_list_result.end()) ==
                 sizeof(expected_decrement_list_result)/sizeof(int)) &&
                std::equal(decrement_list_result.begin(), decrement_list_result.end(),
                           expected_decrement_list_result) );

    // try forward iterators
    int fwd_list_data[] = {-1, -2, -3, -4};
    boost::container::slist<int> fwd_list(fwd_list_data, fwd_list_data + sizeof(fwd_list_data)/sizeof(int));
    padded_range<boost::container::slist<int>::const_iterator, 13, 5> frng(fwd_list.begin(), fwd_list.end());
    std::vector<int> fwd_pad_results(frng.begin(), frng.end());
    int expected_fwd_pad_result[] = {-1,-2,-3,-4,13,13,13,13,13};

    BOOST_TEST( (std::distance(fwd_pad_results.begin(), fwd_pad_results.end()) ==
                 sizeof(expected_fwd_pad_result)/sizeof(int)) &&
                std::equal(fwd_pad_results.begin(), fwd_pad_results.end(), expected_fwd_pad_result) );

    // range with no padding
    padded_range<boost::container::slist<int>::const_iterator, 0, 0> fnp(fwd_list.begin(), fwd_list.end());
    BOOST_TEST( std::distance(fnp.begin(), fnp.end()) == std::distance(fwd_list.begin(), fwd_list.end()));
    BOOST_TEST( std::distance(fnp.begin(), fnp.pad_begin()) == std::distance(fwd_list.begin(), fwd_list.end()));
    BOOST_TEST_EQ( std::distance(fnp.pad_begin(), fnp.end()), 0 );


    // range with padding but no source
    padded_range<boost::container::slist<int>::const_iterator, 7, 5> fns(fwd_list.begin(), fwd_list.begin());
    BOOST_TEST_EQ( std::distance(fns.begin(), fns.end()), 5 );
    BOOST_TEST_EQ( std::distance(fns.begin(), fns.pad_begin()), 0 );
    BOOST_TEST_EQ( std::distance(fns.pad_begin(), fns.end()), 5 );
    

    // range with neither (empty)
    padded_range<boost::container::slist<int>::const_iterator, 0, 0> fnn(fwd_list.begin(), fwd_list.begin());
    BOOST_TEST_EQ( std::distance(fnn.begin(), fnn.end()), 0 );
    BOOST_TEST_EQ( std::distance(fnn.pad_begin(), fnn.end()), 0 );
    BOOST_TEST_EQ( std::distance(fnn.begin(), fnn.pad_begin()), 0 );

    return boost::report_errors();
}
