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
#include <string>

// things we need from other parts of Boost
#include <boost/container/slist.hpp>
#include <boost/detail/lightweight_test.hpp>

// for the class being tested
#include <boost/wave/cpplexer/re2clex/aq.hpp>
#include <boost/wave/cpplexer/re2clex/cpp_re.hpp>

//
// test cases, templated for iterator type so we can check fwd/bidir/randomaccess
//

///////////////////////////////////////////////////////////////////////////////
// basic usage
static std::string basic("struct Foo {};");
template<typename Iter>
void basic_test(Iter beg, Iter end) {
    BOOST_TEST( beg != end );
    BOOST_TEST_EQ( std::distance(beg, end), (std::ptrdiff_t)basic.size() );
    std::advance(beg, basic.size()-1);  // to the last character
    BOOST_TEST_EQ( beg.column, 14 );
    BOOST_TEST_EQ( beg.line, 1 );
    ++beg;  // off the end now
    BOOST_TEST( beg == end );
}

///////////////////////////////////////////////////////////////////////////////
// embedded newlines
static std::string with_nl("struct Foo\n{\r\n};\n");
template<typename Iter>
void nl_test(Iter beg, Iter end) {

    // Test position handling
    // The position of a newline is considered to be the end of the line.

    BOOST_TEST_EQ( std::distance(beg, end), (std::ptrdiff_t)with_nl.size() );
    BOOST_TEST_EQ( beg.column, 1 );
    BOOST_TEST_EQ( beg.line, 1 );
    std::advance(beg, 9);             // just before first newline
    BOOST_TEST_EQ( *beg, 'o' );
    BOOST_TEST_EQ( beg.column, 10 );
    BOOST_TEST_EQ( beg.line, 1 );
    ++beg;                            // right on first newline
    BOOST_TEST_EQ( *beg, '\n' );
    BOOST_TEST_EQ( beg.column, 11 );  // still on same line
    BOOST_TEST_EQ( beg.line, 1 );
    ++beg;                            // right before second newline
    BOOST_TEST_EQ( *beg, '{' );
    BOOST_TEST_EQ( beg.column, 1 );   // we have advanced to the next line
    BOOST_TEST_EQ( beg.line, 2 );
    ++beg;                            // right on the start of CRLF
    BOOST_TEST_EQ( *beg, '\r' );
    BOOST_TEST_EQ( beg.column, 2 );   // still line 2
    BOOST_TEST_EQ( beg.line, 2 );
    ++beg;                            // on nl
    BOOST_TEST_EQ( *beg, '\n' );
    BOOST_TEST_EQ( beg.column, 3 );   // still line 2
    BOOST_TEST_EQ( beg.line, 2 );
    std::advance(beg, 3);
    BOOST_TEST_EQ( *beg, '\n' );      // on final newline
    BOOST_TEST_EQ( beg.column, 3 );
    BOOST_TEST_EQ( beg.line, 3 );
    ++beg;                            // move past
    BOOST_TEST( beg == end );
    
}

///////////////////////////////////////////////////////////////////////////////
// line continuations
//
std::string bsnl("ident\\\nifier\n"       // within identifier (unix)
                 "\"str\\\r\ning\"\n"     // within string (ms-dos)
                 "ident\n"                // no continuation, just normal
                 "regular\\backslash\n"   // just a backslash
                 "cr\\\rbackslash");      // backslash with CR but no LF

template<typename Iter>
void bsnl_test(Iter beg, Iter end) {

    // we remove 5 characters (1 bs/nl, 1 bs/cr/nl ) with this iterator
    BOOST_TEST_EQ( std::distance(beg, end), (std::ptrdiff_t)(bsnl.size() - 5) );

    BOOST_TEST_EQ( std::string(beg, end).substr(0, 11), std::string("identifier\n") );
    BOOST_TEST_EQ( std::string(beg, end).substr(11, 9), std::string("\"string\"\n") );
    BOOST_TEST_EQ( std::string(beg, end).substr(20, 6), std::string("ident\n") );
    BOOST_TEST_EQ( std::string(beg, end).substr(26, 18), std::string("regular\\backslash\n") );
    BOOST_TEST_EQ( std::string(beg, end).substr(44, 13), std::string("cr\\\rbackslash") );

}

///////////////////////////////////////////////////////////////////////////////
int 
main(int argc, char *argv[])
{
    using namespace boost::wave::cpplexer::re2clex;

    basic_test(uchar_wrapper<std::string::iterator>(basic.begin(), basic.end()),
               uchar_wrapper<std::string::iterator>(basic.end(), basic.end()));

    // the same but with a bidirectional iterator underneath
    std::list<char> basic_list(basic.begin(), basic.end());
    basic_test(uchar_wrapper<std::list<char>::iterator>(basic_list.begin(), basic_list.end()),
               uchar_wrapper<std::list<char>::iterator>(basic_list.end(), basic_list.end()));

    // and finally a forward iterator
    boost::container::slist<char> basic_slist(basic.begin(), basic.end());
    basic_test(uchar_wrapper<boost::container::slist<char>::iterator>(basic_slist.begin(), basic_slist.end()),
               uchar_wrapper<boost::container::slist<char>::iterator>(basic_slist.end(), basic_slist.end()));

    // now the embedded newline test
    nl_test(uchar_wrapper<std::string::iterator>(with_nl.begin(), with_nl.end()),
               uchar_wrapper<std::string::iterator>(with_nl.end(), with_nl.end()));

    std::list<char> nl_list(with_nl.begin(), with_nl.end());
    nl_test(uchar_wrapper<std::list<char>::iterator>(nl_list.begin(), nl_list.end()),
               uchar_wrapper<std::list<char>::iterator>(nl_list.end(), nl_list.end()));

    boost::container::slist<char> nl_slist(with_nl.begin(), with_nl.end());
    nl_test(uchar_wrapper<boost::container::slist<char>::iterator>(nl_slist.begin(), nl_slist.end()),
               uchar_wrapper<boost::container::slist<char>::iterator>(nl_slist.end(), nl_slist.end()));

    // line continuations (backslash-newline, and not)
    bsnl_test(uchar_wrapper<std::string::iterator>(bsnl.begin(), bsnl.end()),
              uchar_wrapper<std::string::iterator>(bsnl.end(), bsnl.end()));

    std::list<char> bsnl_list(bsnl.begin(), bsnl.end());
    bsnl_test(uchar_wrapper<std::list<char>::iterator>(bsnl_list.begin(), bsnl_list.end()),
              uchar_wrapper<std::list<char>::iterator>(bsnl_list.end(), bsnl_list.end()));

    boost::container::slist<char> bsnl_slist(bsnl.begin(), bsnl.end());
    bsnl_test(uchar_wrapper<boost::container::slist<char>::iterator>(bsnl_slist.begin(), bsnl_slist.end()),
              uchar_wrapper<boost::container::slist<char>::iterator>(bsnl_slist.end(), bsnl_slist.end()));

    return boost::report_errors();
}
