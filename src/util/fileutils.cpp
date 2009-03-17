/**
 * Copyright 2009, Yahoo!
 *  
 *  Redistribution and use in source and binary forms, with or without
 *  modification, are permitted provided that the following conditions are
 *  met:
 *
 *  1. Redistributions of source code must retain the above copyright
 *     notice, this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in
 *     the documentation and/or other materials provided with the
 *     distribution.
 *  3. Neither the name of Yahoo! nor the names of its
 *     contributors may be used to endorse or promote products derived
 *     from this software without specific prior written permission.
 * 
 *  THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 *  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 *  WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 *  DISCLAIMED. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT,
 *  INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 *  (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 *   SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 *  HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 *  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING
 *  IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 *  POSSIBILITY OF SUCH DAMAGE.
 */

#include "fileutils.hh"

#include <fstream>

#ifdef WIN32
#define PATHSEP		'\\'
#define PATHPARENT	".."
#else
#define PATHSEP		'/'
#define PATHPARENT	".."
#endif

// read the contents of a file into a std::string
std::string file::readFile(const std::string & path)
{
    std::ifstream ifs;
    
    // XXX: win32 i18n issues!  must convert path from UTF8 to wstring

    ifs.open(path.c_str(), std::ios::in);
    if (!ifs.is_open()) return std::string();

    std::string sOut;

    // Optimization: Reduce likelihood of reallocations/copies.
    const int knInitialCapacity = 2048;
    sOut.reserve(knInitialCapacity);
    
    // See Meyers "Effective STL", Item 29.
    copy(std::istreambuf_iterator<char>(ifs),
         std::istreambuf_iterator<char>(),
         back_inserter(sOut));

    return sOut;
}


// get the directory name of a path
std::string file::dirname(const std::string & path)
{
    size_t i = path.rfind(PATHSEP);
    if (i == std::string::npos) {
        return ".";
    } else if (i == path.length() - 1) {
        return path;
    }
    return path.substr(0, i);
}
