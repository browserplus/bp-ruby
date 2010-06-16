#ifndef __UTF8CONVERT_HH__
#define __UTF8CONVERT_HH__

#include <ServiceAPI/bptypes.h>
#include <string>

namespace convert {
#ifdef WIN32
	typedef std::wstring NativePathString;
#else
	typedef std::string NativePathString;
#endif
    std::string toUTF8(const BPPath p);
    NativePathString fromUTF8(const char * utf8Str);
};

#endif
