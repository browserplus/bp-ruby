#include "widetoutf8.hh"

#ifdef WIN32
#include <windows.h>
#endif

std::string
convert::toUTF8(const BPPath p)
{
    std::string rval;
#ifdef WIN32
	LPCWSTR ptr = p;
    // See how much space we need.
    int nChars = WideCharToMultiByte(CP_UTF8, 0, ptr, -1, 0, 0, 0, 0);

    // Do the conversion.
    char* paBuf = new char[nChars];
    int nRtn = WideCharToMultiByte(CP_UTF8, 0, ptr, -1, paBuf, nChars, 0, 0);

    if (nRtn!=0) {
        rval = paBuf;
        delete[] paBuf;
    }
#else
    rval.append(p);
#endif    
    return rval;
}


convert::NativePathString 
convert::fromUTF8(const char * utf8Str)
{
#ifdef WIN32
    std::wstring rval;

	// See how much space we need.
    int nChars = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8Str,
                                     -1, 0, 0);

    // Do the conversion.
    wchar_t* pawBuf = new wchar_t[nChars];
    int nRtn = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, utf8Str,
                                   -1, pawBuf, nChars);
    if (nRtn!=0) {
        rval = pawBuf;
        delete[] pawBuf;
    }
	return rval;
#else
	return std::string(utf8Str);
#endif
}