/*=============================================================================
    Boost.Wave: A Standard compliant C++ preprocessor library

    Re2C based C++ lexer

    http://www.boost.org/

    Copyright (c) 2001-2012 Hartmut Kaiser. Distributed under the Boost
    Software License, Version 1.0. (See accompanying file
    LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#if !defined(CPP_RE_HPP_B76C4F5E_63E9_4B8A_9975_EC32FA6BF027_INCLUDED)
#define CPP_RE_HPP_B76C4F5E_63E9_4B8A_9975_EC32FA6BF027_INCLUDED

#include <boost/wave/wave_config.hpp>
#include <boost/wave/token_ids.hpp>
#include <boost/wave/cpplexer/cpplexer_exceptions.hpp>
#include <boost/wave/cpplexer/re2clex/uchar_wrapper.hpp>

// this must occur after all of the includes and before any code appears
#ifdef BOOST_HAS_ABI_HEADERS
#include BOOST_ABI_PREFIX
#endif

// suppress warnings about dependent classes not being exported from the dll
#ifdef BOOST_MSVC
#pragma warning(push)
#pragma warning(disable : 4251 4231 4660)
#endif

///////////////////////////////////////////////////////////////////////////////
namespace boost {
namespace wave {
namespace cpplexer {
namespace re2clex {

template<typename Iterator> struct Scanner;

#define RE2C_ASSERT BOOST_ASSERT

template<typename Iterator>
int get_one_char(Scanner<Iterator> *s)
{
    if (0 != s->act) {
        RE2C_ASSERT(s->first != 0 && s->last != 0);
        RE2C_ASSERT(s->first <= s->act && s->act <= s->last);
        if (s->act < s->last)
            return *(s->act)++;
    }
    return -1;
}

template<typename Iterator>
std::ptrdiff_t rewind_stream (Scanner<Iterator> *s, int cnt)
{
    if (0 != s->act) {
        RE2C_ASSERT(s->first != 0 && s->last != 0);
        s->act += cnt;
        RE2C_ASSERT(s->first <= s->act && s->act <= s->last);
        return s->act - s->first;
    }
    return 0;
}

template<typename Iterator>
std::size_t get_first_eol_offset(Scanner<Iterator>* s)
{
    if (!AQ_EMPTY(s->eol_offsets))
    {
        return s->eol_offsets->queue[s->eol_offsets->head];
    }
    else
    {
        return (unsigned int)-1;
    }
}

template<typename Iterator>
bool is_backslash(Iterator p, Iterator end, int &len)
{
    if (*p == '\\') {
        len = 1;
        return true;
    }
    else if (*p == '?' && *(p+1) == '?' && (p+2 < end && *(p+2) == '/')) {
        len = 3;
        return true;
    }
    return false;
}

// macro definitions used by re2c

// aliases for variables local to scan()
#define YYCURSOR cursor
#define YYLIMIT  limit
#define YYMARKER marker
#define YYCTXMARKER ctxmarker

// forward iterator-compatible implementation of the RE2C "generic API":
#define YYPEEK()  *YYCURSOR
#define YYSKIP()  ++YYCURSOR
#define YYBACKUP() YYMARKER = YYCURSOR
#define YYRESTORE() YYCURSOR = YYMARKER
#define YYBACKUPCTX() YYCTXMARKER = YYCURSOR
#define YYRESTORECTX() YYCURSOR = YYCTXMARKER

///////////////////////////////////////////////////////////////////////////////
#define BOOST_WAVE_UPDATE_CURSOR()                                            \
    {                                                                         \
        s->line = cursor.line;                                                \
        s->curr_column = cursor.column;                                       \
        s->cur = cursor;                                                      \
        s->lim = limit;                                                       \
        s->ptr = marker;                                                      \
    }                                                                         \
    /**/

///////////////////////////////////////////////////////////////////////////////
#define BOOST_WAVE_RET(i)                                                     \
    {                                                                         \
        BOOST_WAVE_UPDATE_CURSOR()                                            \
        if (s->cur == s->lim)                                                 \
            return T_EOF;     /* may happen for empty files */                \
        return (i);                                                           \
    }                                                                         \
    /**/

///////////////////////////////////////////////////////////////////////////////
//  The scanner function to call whenever a new token is requested
template<typename Iterator>
BOOST_WAVE_DECL boost::wave::token_id scan(Scanner<Iterator> *s)
{
    typedef unsigned char YYCTYPE;

    BOOST_ASSERT(0 != s->error_proc);     // error handler must be given

    uchar_wrapper<Iterator> cursor    (s->tok = s->cur, s->eof, s->column = s->curr_column, s->line);
    uchar_wrapper<Iterator> marker    (s->ptr, s->eof);
    uchar_wrapper<Iterator> ctxmarker (s->ptr, s->eof);
    uchar_wrapper<Iterator> limit     (s->lim, s->eof);

// include the correct Re2C token definition rules
#if BOOST_WAVE_USE_STRICT_LEXER != 0
#include "strict_cpp_re.inc"
#else
#include "cpp_re.inc"
#endif

} /* end of scan */

#undef BOOST_WAVE_RET
#undef YYPEEK
#undef YYSKIP
#undef YYBACKUP
#undef YYRESTORE
#undef YYBACKUPCTX
#undef YYRESTORECTX
#undef YYCURSOR
#undef YYLIMIT
#undef YYMARKER
#undef YYCTXMARKER

// YYMAXFILL *remains defined*

///////////////////////////////////////////////////////////////////////////////
}   // namespace re2clex
}   // namespace cpplexer
}   // namespace wave
}   // namespace boost

#ifdef BOOST_MSVC
#pragma warning(pop)
#endif

// the suffix header occurs after all of the code
#ifdef BOOST_HAS_ABI_HEADERS
#include BOOST_ABI_SUFFIX
#endif

#endif // !defined(CPP_RE_HPP_B76C4F5E_63E9_4B8A_9975_EC32FA6BF027_INCLUDED)
