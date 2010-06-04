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

#include "ServiceGlobals.hh"

// header files from the bp-service-tools project, which makes it
// easier to deal in types that one may transmit across the service
// boundary
#include "bptypeutil.hh"

// an abstraction around ruby 
#include "RubyInterpreter.hh"
#include "RubyHeaders.hh"

#include <map>
#include <string>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>

#ifdef WIN32
#define PATHSEP "\\"
#else
#define PATHSEP "/"
#endif

const BPCFunctionTable * g_bpCoreFunctions;

int
BPPAllocate(void ** instance, const BPString uri, const BPPath serviceDir,
            const BPPath dataDir, const BPPath tempDir, const BPString locale,
            const BPString userAgent, int clientPid)
{
    bp::Map * obj = new bp::Map();
    obj->add("uri", new bp::String(uri));
    obj->add("service_dir", new bp::Path(serviceDir));
    obj->add("data_dir", new bp::Path(dataDir));
    obj->add("temp_dir", new bp::Path(tempDir));
    obj->add("locale", new bp::String(locale));
    obj->add("user_agent", new bp::String(userAgent));
    obj->add("client_pid", new bp::Integer(clientPid));
    *instance = ruby::allocateInstance(obj);
    if (obj) delete obj;
    return 0;
}

void
BPPDestroy(void * instance)
{
    ruby::destroyInstance(instance);
}

void
BPPInvoke(void * instance, const char * funcName,
          unsigned int tid, const BPElement * elem)
{
    bp::Object * obj = bp::Object::build(elem);
    ruby::invoke(instance, funcName, tid, dynamic_cast<bp::Map *>(obj));
    if (obj) delete obj;
}

// file scoped memory representation of the services interface.
static bp::service::Description * s_desc = NULL;


void
BPPShutdown(void)
{
    ruby::shutdown();

    if (s_desc) delete s_desc;
    s_desc = NULL;
}

const BPServiceDefinition *
BPPInitialize(const BPCFunctionTable * coreFunctionTable,
              const BPPath serviceDir,
              const BPPath dependentDir,
              const BPElement * dependentParams)
{
    g_bpCoreFunctions = coreFunctionTable;

    if (dependentDir == NULL) {
        // a description of this corelet.
        static BPServiceDefinition s_rubyInterpreterDef = {
            "RubyInterpreter",
            5, 0, 1,
            "Allows other services to be written in Ruby.",
            0,
            NULL
        };
        return &s_rubyInterpreterDef;
    } else {
        // XXX: use bp::file::Path here

        // serviceDir is the directory the ruby interpreter is installed into

        // depedentDir is the directory of the dependent ruby service

        g_bpCoreFunctions->log(
            BP_INFO,
            "initializing ruby interpreter with service path: %s",
            serviceDir);

        // now let's initialize the ruby Interpreter
        (void) ruby::initialize(serviceDir);

        // the name of the ruby script and path can be extracted from the
        // dependent parameters map 
        bp::Object * params = bp::Object::build(dependentParams);

        if (!params->has("ScriptFile", BPTString)) {
            delete params;
            return NULL;
        }
        std::string path(dependentDir);
        path.append(PATHSEP);
        path.append(((bp::Path *) params->get("ScriptFile"))->value());    

        std::string error;
        s_desc = ruby::loadRubyService(path, error);

        if (s_desc == NULL) {
            g_bpCoreFunctions->log(
                BP_ERROR, "error loading ruby service: %s",
                error.c_str());
            return NULL;
        }
        
        return s_desc->toBPServiceDefinition();
    }
}

const BPPFunctionTable *
BPPGetEntryPoints(void)
{
    static BPPFunctionTable funcTable = {
        BPP_SERVICE_API_VERSION,
        BPPInitialize,
        BPPShutdown,
        BPPAllocate,
        BPPDestroy,
        BPPInvoke,
        NULL,
        NULL,
        NULL
    };

    return &funcTable;
}
