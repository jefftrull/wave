/*=============================================================================
    Boost.Wave: A Standard compliant C++ preprocessor library

    Re2C based C++ lexer

    http://www.boost.org/

    Copyright (c) 2017 Jeffrey E Trull. Distributed under the Boost
    Software License, Version 1.0. (See accompanying file
    LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
=============================================================================*/

#if !defined(UCHAR_WRAPPER_HPP_INCLUDED)
#define UCHAR_WRAPPER_HPP_INCLUDED

#include <boost/wave/wave_config.hpp>

///////////////////////////////////////////////////////////////////////////////
namespace boost {
namespace wave {
namespace cpplexer {
namespace re2clex {

///////////////////////////////////////////////////////////////////////////////
//  Special wrapper class holding the current cursor position
//  and handling line continuation
template<typename WrappedIterator>
struct uchar_wrapper
    : boost::iterator_adaptor<uchar_wrapper<WrappedIterator>,
                              WrappedIterator,          // base
                              boost::use_default,       // value type from base
                              std::forward_iterator_tag // no decrement or random access
                              >
{
    // technically a ForwardIterator requires a default constructor...

    uchar_wrapper (WrappedIterator base_cursor, WrappedIterator eoi,
                   std::size_t column = 1, std::size_t line = 1)
        :   uchar_wrapper::iterator_adaptor_(base_cursor),
            column(column), line(line), eoi(eoi)
    {}

    operator WrappedIterator() const
    {
        return this->base_reference();
    }

    std::size_t column;
    std::size_t line;

private:
    WrappedIterator eoi;     // end of file - and of line continuations

    friend class boost::iterator_core_access;

    void increment()
    {
        // regular position update
        WrappedIterator it = this->base_reference();
        if (it != eoi) {
            if (*it == '\n') {
                // newline belongs to previous line; now we move past it
                column = 1;
                ++line;
            } else {
                ++column;
            }
        }

        it = ++this->base_reference();
        if (it == eoi) {
            return;        // don't look within the padding
        }

        // conventional line continuation with backslash
        if (*it == '\\') {
            // handle line continuation (backslash-newline)
            if ((++it != eoi) && (*it == '\r')) {
                ++it;    // may be \r\n
            }
            if ((it != eoi) && (*it++ == '\n')) {
                // line continuation
                this->base_reference() = it;  // skip it
                column = 1;
                line++;
            }
            return;
        }

        // line continuation via trigraph
        if (*it++ == '?') {
            if ((it != eoi) && (*it++ == '?')) {
                if ((it != eoi) && (*it++ == '/')) {
                    if ((it != eoi) && (*it++ == '\n')) {
                        this->base_reference() = it;  // skip it
                        column = 1;
                        line++;
                    }
                }
            }
        }
    }
};

}}}}

#endif // UCHAR_WRAPPER_HPP_INCLUDED
