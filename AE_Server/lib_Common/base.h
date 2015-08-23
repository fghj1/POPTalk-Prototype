#pragma once

#if _MSC_VER >= 1100
#else
    #define NO_MBSTOWCS_S
    #define NO_WCSTOMBS_S
    #define NO_SPRINTF_S
#endif

namespace frmt
{
    // how many digits we want in width, precision and arg number?
    // please note, that too big number can hit at safety
    enum { eMaxSize = 4 }; //enum _ENUM
    enum { eMaxIntTSize = 128 }; //enum _ENUM

    enum EFormatFlags
    {
        ffLeftAlign         = 1,
        ffRightAlign        = 2,    // (default)
        ffCenterAlign       = (ffLeftAlign | ffRightAlign),

        ffShowSign          = 4,
        ffShowBase          = 8,
        ffShowPadChar       = 16,
        ffShowHexCapital    = 32,
        ffUseLong           = 64,
        ffUse64             = 128,
        ffNoNULL            = 256
    }; //enum EFormatFlags

    template<class charT>
    union format_arg
    {
        // int
        int                 fa_int;
        unsigned int        fa_uint;

        // long
        long                fa_long;
        unsigned long       fa_ulong;
        long long           fa_longlong;
        unsigned long long  fa_ulonglong;

        // double
        double              fa_double;
        long double         fa_longdouble;

        // char
        charT               fa_charT;
        char                fa_char;
        wchar_t             fa_wchar;

        // pointers
        const charT        *fa_cpcharT;
        const char         *fa_cpchar;
        const wchar_t      *fa_cpwchar;
        void               *fa_pointer;
    }; //union format_arg

    // This structure holds everything that is written in format string after % and till type-char
    template<class charT>
    struct format_info
    {
        int         iFlags;         // default: ffRightAlign
        int         iBase;          // default: 10
        charT       ctPadChar;      // default: space (for string), 0 (for numbers)
        int         iWidth;         // default: 0
        int         iPrecision;     // default: 0
        charT       ctTypeChar;
    }; //struct format_info

    template<class charT>
    inline bool is_digit (const charT cct)
    {
        return ('0' <= cct && '9' >= cct);
    }

    template<class charT>
    inline bool is_typechar (const charT cct)
    {
        return ('b' == cct ||   // binary (unsigned int/long)
                'o' == cct ||   // octal (unsigned int/long)
                'i' == cct ||   // decimal (int/long)
                'd' == cct ||   // decimal (long)
                'u' == cct ||   // decimal (unsigned int/long)
                'x' == cct ||   // hex (unsigned int/long)
                'X' == cct ||   // HEX (unsigned int/long)
                'f' == cct ||   // double/long double
                's' == cct ||   // stringT
                'S' == cct ||   // wstring
                'c' == cct ||   // charT
                'C' == cct);    // wchar_t
    }

    template<class charT>
    size_t strlen_t (const charT *cpct)
    {
        size_t iLn = 0;
        while (*cpct++) iLn++;
        return iLn;
    }

    template<class charT>
    const charT *read_flags (const charT *cpct, format_info<charT> *pfi)
    {
        pfi->iFlags     = ffRightAlign;
        pfi->iBase      = 10;
        pfi->ctPadChar  = 0x20;
        pfi->iWidth     = 0;
        pfi->iPrecision = 0;

        while (*cpct)
        {
            switch (*cpct)
            {
                case '+':
                    pfi->iFlags |= ffShowSign;
                    cpct++;   // skip '+'
                    break;

                case '-':
                    pfi->iFlags &= ~(ffRightAlign | ffLeftAlign);
                    pfi->iFlags |= ffLeftAlign;
                    cpct++;   // skip '-'
                    break;

                case '=':
                    pfi->iFlags |= (ffRightAlign | ffLeftAlign);
                    cpct++;   // skip '='
                    break;

                case '0':
                    pfi->ctPadChar = '0';
                    cpct++;   // skip '0'
                    break;

                case '#':
                    pfi->iFlags |= ffShowBase;
                    cpct++;   // skip '#'
                    break;

                case '!':
                    pfi->iFlags |= ffNoNULL;
                    cpct++;   // skip '!'
                    break;

                case 'L':
                    if (ffUseLong == (pfi->iFlags & ffUseLong)) pfi->iFlags |= ffUse64;
                    else pfi->iFlags |= ffUseLong;
                    cpct++;   // skip 'L'
                    break;

                // no proper flag, just return
                default: return cpct;
            } //siwtch
        } //while

        // oops, end of string
        return cpct;
    }

    template<class charT>
    void put_char (std::basic_string<charT, std::char_traits<charT>, std::allocator<charT> >& stOut,
                   const charT *cpct,
                   format_info<charT> *pfi)
    {
        static const charT cctNULL [] = { '(', 'n', 'u', 'l', 'l', ')' };
        if (0 == cpct)
        {
            // do not nothing is user said so :)
            if (ffNoNULL == (pfi->iFlags & ffNoNULL)) return;
            cpct = cctNULL;
        } //if

        size_t iLn = strlen_t<charT> (cpct);

        // if precision is less than actual string length, then we put less of string
        if (pfi->iPrecision && pfi->iPrecision < static_cast<int> (iLn)) iLn = pfi->iPrecision;

        size_t iWidth = iLn; // this is actual number of chars to be inserted
        if (pfi->iWidth > static_cast<int> (iWidth)) iWidth = pfi->iWidth;

        if ((ffShowSign == (pfi->iFlags & ffShowSign)) &&
            ('-' == *cpct || '+' == *cpct))
        {
            stOut += *cpct++; // skip '+' or '-'
            iLn--;
            iWidth--;
        } //if

        if (ffShowBase == (pfi->iFlags & ffShowBase))
        {
            if (8 == pfi->iBase)
            {
                stOut += '0';
                iWidth--;
            } //if
            else if (16 == pfi->iBase)
            {
                iWidth -= 2;
                stOut += '0';
                if (ffShowHexCapital == (pfi->iFlags & ffShowHexCapital)) stOut += 'X';
                else stOut += 'x';
            } //else if
        } //if

        size_t iPos = stOut.length (); // left align

        if (iWidth > iLn)
        {
            // fill entire buffer with pad chars
            stOut.append (iWidth, pfi->ctPadChar);

            if (ffCenterAlign == (pfi->iFlags & ffCenterAlign)) iPos += (iWidth - iLn) / 2;
            else if (ffRightAlign == (pfi->iFlags & ffRightAlign)) iPos += iWidth - iLn;
        } //if

        stOut.replace (iPos, iLn, cpct, iLn);
    }

    // 4 special cases
    template<class charT>
    inline void convert_put (std::basic_string<charT, std::char_traits<charT>, std::allocator<charT> >& stOut,
                             const char *cpc,
                             format_info<charT> *pfi,
                             char /* useless */)
    {
        put_char<charT> (stOut, cpc, pfi);
    }
    template<class charT>
    inline void convert_put (std::basic_string<charT, std::char_traits<charT>, std::allocator<charT> >& stOut,
                             const wchar_t *cpwc,
                             format_info<charT> *pfi,
                             wchar_t /* useless */)
    {
        put_char<charT> (stOut, cpwc, pfi);
    }
    // convert char-to-wchar
    template<class charT>
    void convert_put (std::basic_string<charT, std::char_traits<charT>, std::allocator<charT> >& stOut,
                      const char *cpc,
                      format_info<charT> *pfi,
                      wchar_t /* useless */)
    {
        size_t iStrLn = strlen_t<char> (cpc);
        size_t iLn = 0;
    #ifdef NO_MBSTOWCS_S
        iLn = mbstowcs (0, cpc, iStrLn);
    #else
        errno_t err = mbstowcs_s (&iLn, 0, 0, cpc, iStrLn);
    #endif //NO_MBSTOWCS_S

        if (0 <= iLn) put_char<charT> (stOut, 0, pfi);
        else
        {
            wchar_t *pwc = new wchar_t [iLn + 1];
    #ifdef NO_MBSTOWCS_S
            iLn = mbstowcs (pwc, cpc, iStrLn);
            if (iLn <= 0)
            {
                delete[] pwc;
                return;
            } //if
            *(pwc + iLn) = '\0';
    #else
            err = mbstowcs_s (&iLn, pwc, iLn + 1, cpc, iStrLn);
    #endif //NO_MBSTOWCS_S
            put_char<charT> (stOut, pwc, pfi);
            delete[] pwc;
        } //else
    }
    // convert wchar-to-char
    template<class charT>
    void convert_put (std::basic_string<charT, std::char_traits<charT>, std::allocator<charT> >& stOut,
                      const wchar_t *cpwc,
                      format_info<charT> *pfi,
                      char /* useless */)
    {
        size_t iStrLn = strlen_t<wchar_t> (cpwc);
        size_t iLn = 0;
    #ifdef NO_WCSTOMBS_S
        iLn = wcstombs (0, cpwc, iStrLn);
    #else
        errno_t err = wcstombs_s (&iLn, 0, 0, cpwc, iStrLn);
    #endif //NO_WCSTOMBS_S

        if (0 == iLn) put_char<charT> (stOut, 0, pfi);
        else
        {
            char *pc = new char [iLn + 1];
        #ifdef NO_WCSTOMBS_S
            iLn = wcstombs (pc, cpwc, iStrLn);
            if (iLn <= 0)
            {
                delete[] pc;
                return;
            } //if
            *(pc + iLn) = '\0';
        #else
            err = wcstombs_s (&iLn, pc, iLn + 1, cpwc, iStrLn);
        #endif //NO_WCSTOMBS_S
            put_char<charT> (stOut, pc, pfi);
            delete[] pc;
        } //else
    }

    template<class charT, class intT>
    void put_int (std::basic_string<charT, std::char_traits<charT>, std::allocator<charT> >& stOut,
                  intT it,
                  format_info<charT> *pfi)
    {
        // create array of charT, but don't fill it with 0's
        register charT ctData [eMaxIntTSize];
        register charT *pct = &ctData [eMaxIntTSize - 1];

        *pct = 0;

        if (0 == it)
        {
            // do not nothing is user said so :)
            if (ffNoNULL == (pfi->iFlags & ffNoNULL)) return;
            *(--pct) = '0';
            if (ffShowSign == (pfi->iFlags & ffShowSign)) *(--pct) = '+';
        } //if
        else
        {
            register int i;
            bool bNeg = (it < 0);

            int iHexChar = -10; // 'a' (97) - 10, because: if (i >= 10)...
            if (ffShowHexCapital == (pfi->iFlags & ffShowHexCapital)) iHexChar += 'A';
            else iHexChar += 'a';

            while (it)
            {
                i = static_cast<int> (it % pfi->iBase);
                if (bNeg) i = -i;

                if (i >= 10) *(--pct) = i + iHexChar;
                else *(--pct) = i + '0';

                it /= pfi->iBase;
            } //while

            if (bNeg) *(--pct) = '-';
            else if (ffShowSign == (pfi->iFlags & ffShowSign)) *(--pct) = '+';
        } //else

        put_char<charT> (stOut, pct, pfi);
    }
} //namespace frmt
