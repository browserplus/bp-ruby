/**
 * INSERT BSD LICENSE HERE
 */

#include <stdio.h>
#include <stdlib.h>

#include "ServiceAPI/bpcfunctions.h"
#include "ServiceAPI/bpdefinition.h"
#include "ServiceAPI/bperror.h"
#include "ServiceAPI/bppfunctions.h"
#include "ServiceAPI/bptypes.h"

#include <map>
#include <string>

const BPCFunctionTable * g_bpCoreFunctions;
/*
// an stl map which keeps track of attachments & bindings 
static std::map<unsigned int, RubyCorelet *> s_attachments;
static std::list<RubyCorelet *> s_rubyCorelets;
*/

// at allocation time, we read and evaluate the ruby script in a
// clean binding
int
BPPAllocate(void ** instance, unsigned int attachID,
            const BPElement * contextMap)
{
    int rc = 0;
/*

    // this means we must new an object in the correct evaluation context
    *instance = NULL;

    if (attachID == 0) {
        // XXX: for now we don't support the native interface so good.
    } else {
        // locking please!
        s_lock->lock();
        std::map<unsigned int, RubyCorelet *>::iterator it;
        it = s_attachments.find(attachID);
        if (it == s_attachments.end()) {
            g_bpCoreFunctions->log(
                BP_ERROR,
                "(RubyInterpreter) BPPAllocate called with invalid attachment: %u",
                attachID);
            rc = 1;
        } else {
            bp::Object * ctx = bp::Object::build(contextMap);
            *instance = (void *) it->second->instance(
                dynamic_cast<bp::Map *>(ctx));
            delete ctx;
        }
        s_lock->unlock();
    }
*/
    return rc;
}

void
BPPDestroy(void * instance)
{
/*
    if (instance != NULL) {
        delete (RubyCoreletInstance *) instance;
    }
*/
}

void
BPPShutdown(void)
{
/*
    // delete all ruby corelets
    std::list<RubyCorelet *>::iterator i;
    for (i = s_rubyCorelets.begin(); i != s_rubyCorelets.end(); i++)
        delete *i;
    
    RubyEvaluatorShutdown();
    delete s_lock;
*/
}

void
BPPInvoke(void * instance, const char * funcName,
          unsigned int tid, const BPElement * elem)
{
/*
    if (instance != NULL) {
        RubyCoreletInstance * cinstance = (RubyCoreletInstance *) instance;
        bp::Map * args = NULL;
        args = (bp::Map *) bp::Object::build(elem);
        cinstance->invoke(funcName, tid, args);
        delete args;
    } else {
        g_bpCoreFunctions->postError(tid, BPE_NOT_IMPLEMENTED, NULL);
    }
*/
}

// a description of this corelet.
BPCoreletDefinition s_rubyInterpreterDef = {
    "RubyInterpreter",
    5, 0, 0,
    "Allows other services to be written in Ruby.",
    0,
    NULL
};

const BPCoreletDefinition *
BPPAttach(unsigned int attachID, const BPElement * paramMap)
{
/*
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

    bp::file::appendPath(path,
                         ((bp::Path *) params->get("ScriptFile"))->value());

    // see if we've already loaded this thing
    RubyCorelet * rc = NULL;
    
    s_lock->lock();    
    std::list<RubyCorelet *>::iterator i;
    for (i = s_rubyCorelets.begin(); i != s_rubyCorelets.end(); i++) {
        if (!((*i)->pathToRubyFile().compare(path))) {
            // corelet already loaded
            rc = *i;
            def = rc->getDefinition();
        }
    }
        
    if (rc == NULL) {
        rc = new RubyCorelet;        
        def = rc->loadCoreletFile(path);
        if (def == NULL) {
            delete rc;
            rc = NULL;
        } else {
            s_rubyCorelets.push_back(rc);            
        }
    }
    
    // now allocate a ruby corelet
    if (rc != NULL) {
        assert(def != NULL);
        s_attachments[attachID] = rc;
    }

    s_lock->unlock();

    return def;
*/
    return NULL;
}

void
BPPDetach(unsigned int attachID)
{
/*
    // find and remove the attachment

    // locking please!
    s_lock->lock();
    std::map<unsigned int, RubyCorelet *>::iterator it;
    it = s_attachments.find(attachID);
    if (it == s_attachments.end()) {
        g_bpCoreFunctions->log(
            BP_ERROR,
            "(RubyInterpreter) BPDetach called with invalid attachment id: %u",
            attachID);
    } else {
        s_attachments.erase(it);
    }
    s_lock->unlock();
*/
}

const BPCoreletDefinition *
BPPInitialize(const BPCFunctionTable * bpCoreFunctions,
              const BPElement * parameterMap)
{
/*
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
    s_lock = new bp::sync::Mutex;
    RubyEvaluatorStartup(path);
    delete obj;
*/
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
