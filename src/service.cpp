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
 *
 */

#include <stdio.h>
#include <stdlib.h>

// header files from the BrowserPlus SDK
#include "ServiceAPI/bpcfunctions.h"
#include "ServiceAPI/bpdefinition.h"
#include "ServiceAPI/bperror.h"
#include "ServiceAPI/bppfunctions.h"
#include "ServiceAPI/bptypes.h"

// header files from the bp-service-tools project, which makes it
// easier to deal in types that one may transmit across the service
// boundary
#include "bptypeutil.hh"

// an abstraction around ruby 
#include "RubyInterpreter.hh"

#include <map>
#include <string>
#include <iostream>

#ifdef WIN32
#define PATHSEP "\\"
#else
#define PATHSEP "/"
#endif

const BPCFunctionTable * g_bpCoreFunctions;


int
BPPAllocate(void ** instance, unsigned int attachID,
            const BPElement * contextMap)
{
    int rc = 0;
    // XXX: write me!
    return rc;
}

void
BPPDestroy(void * instance)
{
    // XXX: write me!
}

void
BPPInvoke(void * instance, const char * funcName,
          unsigned int tid, const BPElement * elem)
{
    // XXX: write me!
}

void
BPPShutdown(void)
{
    ruby::shutdown();
}

// a description of this corelet.
BPCoreletDefinition s_rubyInterpreterDef = {
    "RubyInterpreter",
    5, 0, 0,
    "Allows other services to be written in Ruby.",
    0,
    NULL
};

// file scoped memory representation of the services interface.
static bp::service::Description * s_desc = NULL;

const BPCoreletDefinition *
BPPAttach(unsigned int attachID, const BPElement * paramMap)
{
    const BPCoreletDefinition * def = NULL;

    // the name of the ruby script and path can be extracted from the
    // parameter map 
    bp::Object * obj = bp::Object::build(paramMap);

    // first get the path
    if (!obj->has("CoreletDirectory", BPTString)) {
        delete obj;
        return NULL;
    }
    std::string path;
    path.append(((bp::Path *) obj->get("CoreletDirectory"))->value());

    // now get the script name
    if (!obj->has("Parameters", BPTMap)) {
        delete obj;
        return NULL;
    }

    bp::Map * params = (bp::Map *) obj->get("Parameters");
    
    if (!params->has("ScriptFile", BPTString)) {
        delete obj;
        return NULL;
    }

    path.append(PATHSEP);
    path.append(((bp::Path *) params->get("ScriptFile"))->value());    

    std::string error;
    s_desc = ruby::loadRubyService(path, error);

    if (s_desc == NULL) {
        std::cerr << "error loading ruby service: " << error
                  << std::endl;
        return NULL;
    }

    return s_desc->toBPCoreletDefinition();
}

void
BPPDetach(unsigned int attachID)
{
    if (s_desc) delete s_desc;
    s_desc = NULL;
}

const BPCoreletDefinition *
BPPInitialize(const BPCFunctionTable * bpCoreFunctions,
              const BPElement * parameterMap)
{
    // the name of the ruby script and path can be extracted from the
    // parameter map 
    bp::Object * obj = bp::Object::build(parameterMap);

    // first get the path
    if (!obj->has("CoreletDirectory", BPTString)) {
        delete obj;
        return NULL;
    }

    std::string path(((bp::String *) obj->get("CoreletDirectory"))->value());

    g_bpCoreFunctions = bpCoreFunctions;

    delete obj;

    // this will go in the BrowserPlusCore log file at info level.  nice.
    std::cout << "initializing ruby interpreter with service path: "
              << path << std::endl;

    // now let's initialize the ruby Interpreter
    (void) ruby::initialize(path);

    return &s_rubyInterpreterDef;
}

/** and finally, declare the entry point to the corelet */
BPPFunctionTable funcTable = {
    BPP_CORELET_API_VERSION,
    BPPInitialize,
    BPPShutdown,
    BPPAllocate,
    BPPDestroy,
    BPPInvoke,
    BPPAttach,
    BPPDetach
};

const BPPFunctionTable *
BPPGetEntryPoints(void)
{
    return &funcTable;
}
