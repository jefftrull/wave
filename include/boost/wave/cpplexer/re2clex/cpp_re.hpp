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
void adjust_eol_offsets(Scanner<Iterator>* s, std::size_t adjustment)
{
    aq_queue q;
    std::size_t i;

    if (!s->eol_offsets)
        s->eol_offsets = aq_create();

    q = s->eol_offsets;

    if (AQ_EMPTY(q))
        return;

    i = q->head;
    while (i != q->tail)
    {
        if (adjustment > q->queue[i])
            q->queue[i] = 0;
        else
            q->queue[i] -= adjustment;
        ++i;
        if (i == q->max_size)
            i = 0;
    }
    if (adjustment > q->queue[i])
        q->queue[i] = 0;
    else
        q->queue[i] -= adjustment;
}

template<typename Iterator>
int count_backslash_newlines(Scanner<Iterator> *s, Iterator cursor)
{
    std::size_t diff, offset;
    int skipped = 0;

    /* figure out how many backslash-newlines skipped over unknowingly. */
    diff = cursor - s->bot;
    offset = get_first_eol_offset(s);
    while (offset <= diff && offset != (unsigned int)-1)
    {
        skipped++;
        aq_pop(s->eol_offsets);
        offset = get_first_eol_offset(s);
    }
    return skipped;
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

#define BOOST_WAVE_BSIZE     196608

template<typename Iterator>
Iterator fill(Scanner<Iterator> *s, Iterator cursor)
{
    using namespace std;    // some systems have memcpy etc. in namespace std
    typedef typename std::iterator_traits<Iterator>::value_type uchar;
    if(!s->eof)
    {
        Iterator p;
        std::ptrdiff_t cnt = s->tok - s->bot;
        if(cnt)
        {
            if (NULL == s->lim)
                s->lim = s->top;
            memmove(s->bot, s->tok, s->lim - s->tok);
            s->tok = s->cur = s->bot;
            s->ptr -= cnt;
            cursor -= cnt;
            s->lim -= cnt;
            adjust_eol_offsets(s, cnt);
        }

        if((s->top - s->lim) < BOOST_WAVE_BSIZE)
        {
            Iterator buf = (Iterator) malloc(((s->lim - s->bot) + BOOST_WAVE_BSIZE)*sizeof(uchar));
            if (buf == 0)
            {
                (*s->error_proc)(s, lexing_exception::unexpected_error,
                    "Out of memory!");

                /* get the scanner to stop */
                *cursor = 0;
                return cursor;
            }

            memmove(buf, s->tok, s->lim - s->tok);
            s->tok = s->cur = buf;
            s->ptr = &buf[s->ptr - s->bot];
            cursor = &buf[cursor - s->bot];
            s->lim = &buf[s->lim - s->bot];
            s->top = &s->lim[BOOST_WAVE_BSIZE];
            free(s->bot);
            s->bot = buf;
        }

        if (s->act != 0) {
            cnt = s->last - s->act;
            if (cnt > BOOST_WAVE_BSIZE)
                cnt = BOOST_WAVE_BSIZE;
            memmove(s->lim, s->act, cnt);
            s->act += cnt;
            if (cnt != BOOST_WAVE_BSIZE)
            {
                s->eof = &s->lim[cnt]; *(s->eof)++ = '\0';
            }
        }

        /* backslash-newline erasing time */

        /* first scan for backslash-newline and erase them */
        for (p = s->lim; p < s->lim + cnt - 2; ++p)
        {
            int len = 0;
            if (is_backslash(p, s->lim + cnt, len))
            {
                if (*(p+len) == '\n')
                {
                    int offset = len + 1;
                    memmove(p, p + offset, s->lim + cnt - p - offset);
                    cnt -= offset;
                    --p;
                    aq_enqueue(s->eol_offsets, p - s->bot + 1);
                }
                else if (*(p+len) == '\r')
                {
                    if (*(p+len+1) == '\n')
                    {
                        int offset = len + 2;
                        memmove(p, p + offset, s->lim + cnt - p - offset);
                        cnt -= offset;
                        --p;
                    }
                    else
                    {
                        int offset = len + 1;
                        memmove(p, p + offset, s->lim + cnt - p - offset);
                        cnt -= offset;
                        --p;
                    }
                    aq_enqueue(s->eol_offsets, p - s->bot + 1);
                }
            }
        }

        /* FIXME: the following code should be fixed to recognize correctly the
                  trigraph backslash token */

        /* check to see if what we just read ends in a backslash */
        if (cnt >= 2)
        {
            uchar last = s->lim[cnt-1];
            uchar last2 = s->lim[cnt-2];
            /* check \ EOB */
            if (last == '\\')
            {
                int next = get_one_char(s);
                /* check for \ \n or \ \r or \ \r \n straddling the border */
                if (next == '\n')
                {
                    --cnt; /* chop the final \, we've already read the \n. */
                    aq_enqueue(s->eol_offsets, cnt + (s->lim - s->bot));
                }
                else if (next == '\r')
                {
                    int next2 = get_one_char(s);
                    if (next2 == '\n')
                    {
                        --cnt; /* skip the backslash */
                    }
                    else
                    {
                        /* rewind one, and skip one char */
                        rewind_stream(s, -1);
                        --cnt;
                    }
                    aq_enqueue(s->eol_offsets, cnt + (s->lim - s->bot));
                }
                else if (next != -1) /* -1 means end of file */
                {
                    /* next was something else, so rewind the stream */
                    rewind_stream(s, -1);
                }
            }
            /* check \ \r EOB */
            else if (last == '\r' && last2 == '\\')
            {
                int next = get_one_char(s);
                if (next == '\n')
                {
                    cnt -= 2; /* skip the \ \r */
                }
                else
                {
                    /* rewind one, and skip two chars */
                    rewind_stream(s, -1);
                    cnt -= 2;
                }
                aq_enqueue(s->eol_offsets, cnt + (s->lim - s->bot));
            }
            /* check \ \n EOB */
            else if (last == '\n' && last2 == '\\')
            {
                cnt -= 2;
                aq_enqueue(s->eol_offsets, cnt + (s->lim - s->bot));
            }
        }

        s->lim += cnt;
        if (s->eof) /* eof needs adjusting if we erased backslash-newlines */
        {
            s->eof = s->lim;
            *(s->eof)++ = '\0';
        }
    }
    return cursor;
}

///////////////////////////////////////////////////////////////////////////////
//  Special wrapper class holding the current cursor position
template<typename WrappedIterator>
struct uchar_wrapper
    : boost::iterator_adaptor<uchar_wrapper<WrappedIterator>,
                              WrappedIterator>
{
    uchar_wrapper (WrappedIterator base_cursor, std::size_t column = 1)
        :   uchar_wrapper::iterator_adaptor_(base_cursor), column(column)
    {}

    operator WrappedIterator() const
    {
        return this->base_reference();
    }

    std::size_t column;

private:
    friend class boost::iterator_core_access;

    void increment()
    {
        ++this->base_reference();
        ++column;
    }

};

// macro definitions used by re2c

// aliases for variables local to scan()
#define YYCURSOR cursor
#define YYLIMIT  limit
#define YYMARKER marker
#define YYCTXMARKER ctxmarker

// forward iterator-compatible implementation of the RE2C "generic API":
#define YYLESSTHAN(n) (std::distance(YYCURSOR, YYLIMIT) < n)
#define YYPEEK()  *YYCURSOR
#define YYSKIP()  ++YYCURSOR
#define YYBACKUP() YYMARKER = YYCURSOR
#define YYRESTORE() YYCURSOR = YYMARKER
#define YYBACKUPCTX() YYCTXMARKER = YYCURSOR
#define YYRESTORECTX() YYCURSOR = YYCTXMARKER
#define YYFILL(n)                                                             \
    {                                                                         \
        cursor = uchar_wrapper<unsigned char *>(fill<unsigned char *>(s, cursor), cursor.column);               \
        limit = uchar_wrapper<unsigned char *> (s->lim);                                       \
    }                                                                         \
    /**/

///////////////////////////////////////////////////////////////////////////////
#define BOOST_WAVE_UPDATE_CURSOR()                                            \
    {                                                                         \
        s->line += count_backslash_newlines<unsigned char *>(s, cursor);                       \
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
        if (s->cur > s->lim)                                                  \
            return T_EOF;     /* may happen for empty files */                \
        return (i);                                                           \
    }                                                                         \
    /**/

///////////////////////////////////////////////////////////////////////////////
//  The scanner function to call whenever a new token is requested
template<typename Iterator>
BOOST_WAVE_DECL boost::wave::token_id scan(Scanner<Iterator> *s)
{
    typedef typename std::iterator_traits<Iterator>::value_type YYCTYPE;

    BOOST_ASSERT(0 != s->error_proc);     // error handler must be given

    uchar_wrapper<Iterator> cursor    (s->tok = s->cur, s->column = s->curr_column);
    uchar_wrapper<Iterator> marker    (s->ptr);
    uchar_wrapper<Iterator> ctxmarker (s->ptr);
    uchar_wrapper<Iterator> limit     (s->lim);

// include the correct Re2C token definition rules
#if BOOST_WAVE_USE_STRICT_LEXER != 0
#include "strict_cpp_re.inc"
#else
#include "cpp_re.inc"
#endif

} /* end of scan */

#undef BOOST_WAVE_RET
#undef YYLESSTHAN
#undef YYPEEK
#undef YYSKIP
#undef YYBACKUP
#undef YYRESTORE
#undef YYBACKUPCTX
#undef YYRESTORECTX
#undef YYFILL
#undef YYCURSOR
#undef YYLIMIT
#undef YYMARKER
#undef YYCTXMARKER

#undef BOOST_WAVE_BSIZE


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
