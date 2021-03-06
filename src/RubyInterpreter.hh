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

#ifndef __RUBYINTERPRETER_HH__
#define __RUBYINTERPRETER_HH__

#include "ServiceAPI/bptypes.h"
#include "bpservicedescription.hh"

#include <string>

namespace ruby {
    // intialize the ruby interpreter, given the path to this service.
    // this will call all required initialization routines and
    // will correctly populate load paths.
    void initialize(const std::string & pathToRubyServiceDataDir);

    // given a path to a entry point ruby file, load the file and
    // extract a description.
    // on error, NULL is returned and a human readable error is returned
    // in the oError output param
    bp::service::Description *
        loadRubyService(const std::string & pathToRubyFile,
                        std::string & oError);
    
    void * allocateInstance(const bp::Map * context);

    void invoke(void * instance, const char * funcName,
                unsigned int tid, bp::Map * arguments);

    void destroyInstance(void * instance);

    // shutdown the ruby interpreter, freeing all possible resources.
    void shutdown(void);
}

#endif
