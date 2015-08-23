#pragma once

/*
    %[<|><{>][flags][:padchar][width][.precision]type-char[<|><}>]

    [flags] optional flag.
    '-':  left alignment (default is right alignment)
    '=':  centered alignment
    '+':  show sign
    '!':  don't print int that is *0* or string that is *null*
    'L':  use long instead of int
    'LL': use long long instead of long
    '0':  pad with pad char (if pad char is not specified, then fill with 0's)
    '#':  show base. o- 0, x- 0x, X- 0X

    [:padchar] optional flag. Default value depends on [flags].
    [width] optional flag. Specifies a minimal width for the string resulting form the conversion.
    [.precision] optional flag. When outputting a floatting type number, it sets the maximum number of digits.
                 When used with type-char s or S the conversion string is truncated to the precision first chars.
    type-char:
    'b':            binary output
    'o':            octal output
    'i', 'u', 'd':  decimal output (signed, unsigned)
    'x':            hexadecimal output ('X' upper case)
    'f':            fixed float format
    's', 'S':       string output
    'c', 'C':       char output
    '%':            print '%'
*/

#include <string>
#include <vector>
#include <stdarg.h>
#include "base.h"

namespace frmt
{
    template<class charT>
    size_t vformat (std::basic_string<charT, std::char_traits<charT>, std::allocator<charT> >& stOut,
                    const charT *cpctFormat,
                    va_list vargs)
    {
    // BAD STYLE - CODE REDUSE :)
    #define UNSIGNED_PRINT                                                                          \
        if (ffUseLong != (fi.iFlags & ffUseLong))                                                   \
            put_int<charT, unsigned int> (stOut, va_arg (vargs, unsigned int), &fi);                \
        else if (ffUse64 == (fi.iFlags & ffUse64))                                                  \
            put_int<charT, unsigned long long> (stOut, va_arg (vargs, unsigned long long), &fi);    \
        else                                                                                        \
            put_int<charT, unsigned long> (stOut, va_arg (vargs, unsigned long), &fi);

        // real code begins here
        // ------- ------- ------- ------- ------- ------- -------
        //stOut = "";
        stOut.clear (); // somehow slower?

        register const charT *cpct = cpctFormat;

        while (*cpct)
        {
            if ('%' == *cpct)
            {
                cpct++;  // skip '%'

                // end of string, this shouldn't happen on normal format
                if ('\0' == *cpct) return (cpct - cpctFormat);

                // skip this section, this is not format
                if ('%' == *cpct)
                {
                    stOut += *cpct++;  // skip '%'
                    continue;
                } //if

                int iCover = 0; // 0: none, 1: '{', 2: '|'
                if ('{' == *cpct || '|' == *cpct)
                {
                    if ('{' == *cpct) iCover = 1;
                    else iCover = 2;
                    cpct++; // skip '{' or '|'
                } //if

                format_info<charT> fi;

                // set default format info and read flags if there are any
                cpct = read_flags (cpct, &fi);
                if ('\0' == *cpct) return (cpct - cpctFormat);

                // is it a pad char?
                if (':' == *cpct)
                {
                    cpct++; // skip ':'
                    if ('\0' == *cpct) return (cpct - cpctFormat);
                    fi.ctPadChar = *cpct++; // read and skip pad char
                    if ('\0' == *cpct) return (cpct - cpctFormat);
                } //if

                // is it width?
                if ('*' == *cpct)
                {
                    cpct++; // skip '*'
                    fi.iWidth = va_arg (vargs, int);
                } //if
                else if (is_digit (*cpct))
                {
                    char cInt [eMaxSize] = { 0 };
                    for (int i = 0; i < eMaxSize && is_digit (*cpct); i++, cpct++)
                        cInt [i] = static_cast<char> (*cpct);

                    // it is bigger number than expected or it is end of string
                    if ('\0' == *cpct || is_digit (*cpct)) return (cpct - cpctFormat);
                    fi.iWidth = atoi (cInt);
                } //else if

                // last, but not least moment: the precision
                if ('.' == *cpct)
                {
                    cpct++; // skip '.'
                    // end of string?
                    if ('\0' == *cpct) return (cpct - cpctFormat);

                    if ('*' == *cpct)
                    {
                        cpct++; // skip '*'
                        fi.iPrecision = va_arg (vargs, int);
                    } //if
                    else
                    {
                        char cInt [eMaxSize] = { 0 };
                        for (int i = 0; i < eMaxSize && is_digit (*cpct); i++, cpct++)
                            cInt [i] = static_cast<char> (*cpct);

                        // it is bigger number than expected or it is end of string
                        if ('\0' == *cpct || is_digit (*cpct)) return (cpct - cpctFormat);
                        fi.iPrecision = atoi (cInt);
                    } //else
                } //if

                // at last, check for type-char
                if (!is_typechar (*cpct)) return (cpct - cpctFormat);
                fi.ctTypeChar = *cpct++;

                // remove user's jokes R.C. :)
                // add additional stuff to format_info
                switch (fi.ctTypeChar)
                {
                    case 'b':
                        fi.iBase   = 2;
                        fi.iFlags &= ~(ffShowSign | ffShowBase);
                        UNSIGNED_PRINT;
                        break;

                   case 'o':
                        fi.iBase   = 8;
                        fi.iFlags &= ~ffShowSign;
                        UNSIGNED_PRINT;
                        break;

                    case 'x':
                        fi.iBase   = 16;
                        fi.iFlags &= ~ffShowSign;
                        UNSIGNED_PRINT;
                        break;

                    case 'X':
                        fi.iBase   = 16;
                        fi.iFlags &= ~ffShowSign;
                        fi.iFlags |=  ffShowHexCapital;
                        UNSIGNED_PRINT;
                        break;

                    case 'u': UNSIGNED_PRINT; break;

                    case 'i':
                    case 'd':
                    {
                        if (ffUseLong != (fi.iFlags & ffUseLong))
                            put_int<charT, int> (stOut, va_arg (vargs, int), &fi);
                        else if (ffUse64 == (fi.iFlags & ffUse64))
                            put_int<charT, long long> (stOut, va_arg (vargs, long long), &fi);
                        else 
                            put_int<charT, long> (stOut, va_arg (vargs, long), &fi);

                        break;
                    } //case

                    case 's':
                        fi.iFlags &= ~(ffShowSign | ffShowBase | ffUse64 | ffUseLong);
                        put_char (stOut, va_arg (vargs, const charT *), &fi);
                        break;

                    case 'S':
                        fi.iFlags &= ~(ffShowSign | ffShowBase | ffUse64 | ffUseLong);
                        convert_put<charT> (stOut, va_arg (vargs, const wchar_t *), &fi, fi.ctPadChar);
                        break;

                    case 'c':
                    {
                        fi.iFlags &= ~(ffShowSign | ffShowBase | ffUse64 | ffUseLong);

                        union format_arg<charT> farg;
                        farg.fa_cpcharT = va_arg (vargs, const charT *);

                        charT ct [2] = { farg.fa_charT, 0 };
                        put_char (stOut, ct, &fi);
                        break;
                    } //case

                    case 'C':
                    {
                        fi.iFlags &= ~(ffShowSign | ffShowBase | ffUse64 | ffUseLong);

                        union format_arg<charT> farg;
                        farg.fa_cpwchar = va_arg (vargs, const wchar_t *);

                        wchar_t ct [2] = { farg.fa_wchar, 0 };
                        convert_put<charT> (stOut, ct, &fi, fi.ctPadChar);
                        break;
                    } //case

                    case 'f':
                    {
                        fi.iFlags &= ~(ffShowBase | ffUse64);
                        char cFloat [64];

                        if (ffUseLong == (fi.iFlags & ffUseLong))
                        {
                            if (fi.iPrecision && fi.iPrecision < 64)
                    #if !defined(NO_SPRINTF_S)
                                sprintf_s<64> (cFloat, "%.*f", fi.iPrecision, va_arg (vargs, long double));
                    #else
                                sprintf (cFloat, "%.f", fi.iPrecision, va_arg (vargs, long double));
                    #endif // !NO_PRINTF_S
                            else
                    #if !defined(NO_SPRINTF_S)
                                sprintf_s<64> (cFloat, "%f", va_arg (vargs, long double));
                    #else
                                sprintf (cFloat, "%f", va_arg (vargs, long double));
                    #endif // !NO_SPRINTF_S
                        } //if
                        else
                        {
                            if (fi.iPrecision && fi.iPrecision < 64)
                    #if !defined(NO_SPRINTF_S)
                                sprintf_s<64> (cFloat, "%.*f", fi.iPrecision, va_arg (vargs, double));
                    #else
                                sprintf (cFloat, "%.*f", fi.iPrecision, va_arg (vargs, double));
                    #endif // !NO_SPRINTFF_S
                            else
                    #if !defined(NO_SPRINTF_S)
                                sprintf_s<64> (cFloat, "%f", va_arg (vargs, double));
                    #else
                                sprintf (cFloat, "%f", va_arg (vargs, double));
                    #endif // !NO_SPRINTF_S
                        } //else

                        convert_put<charT> (stOut, cFloat, &fi, fi.ctPadChar);
                        break;
                    } //case
                } //switch

                if (iCover)
                {
                    if (1 == iCover && '}' != *cpct) return (cpct - cpctFormat);
                    if (2 == iCover && '|' != *cpct) return (cpct - cpctFormat);

                    cpct++; // skip '}' or '|'
                } //if
            } //if
            else stOut += *cpct++;
        } //while

        return 0;
    }
} //namespace frmt
