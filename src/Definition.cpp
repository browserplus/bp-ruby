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

/**
 * code to extract a corelet definition from a ruby corelet
 */

#include "Definition.hh"
#include "RubyUtils.hh"
#include "RubyHeaders.hh"

#include <sstream> 
const char * ruby::BP_GLOBAL_DEF_SYM = "$BrowserPlusEntryPointClass";
#define BP_EXTERNAL_REP_METHOD "to_service_description"

bool extractString(VALUE hash, const char * key, std::string & where)
{
    where.clear();
    VALUE rkey = rb_str_new2(key);
    VALUE val = rb_hash_aref(hash, rkey);
    if (rb_type(val) != T_STRING) return false;
    where.append(RSTRING_PTR(val));
    return true;
}

bool extractNumber(VALUE hash, const char * key, unsigned int * where)
{
    VALUE rkey = rb_str_new2(key);
    VALUE val = rb_hash_aref(hash, rkey);
    if (rb_type(val) != T_FIXNUM) return false;
    *where = NUM2UINT(val);
    return true;
}

bool extractBool(VALUE hash, const char * key, bool * where)
{
    VALUE rkey = rb_str_new2(key);
    VALUE val = rb_hash_aref(hash, rkey);
    if (rb_type(val) == T_TRUE) {
        *where = true;
    } else if (rb_type(val) == T_FALSE) {
        *where = false;
    } else {
        return false;
    }
    return true;
}

bool processArgument(VALUE hash, bp::service::Function & f,
                     std::string & verboseError)
{
    bp::service::Argument a;
    std::string s;
    
    if (!extractString(hash, "name", s)) {
        verboseError.append("'name' missing from argument definition\n");
        return false;
    }
    a.setName(s.c_str());
    
    if (!extractString(hash, "documentation", s)) {
        verboseError.append("'documentation' missing from ");
        verboseError.append(a.name());
        verboseError.append(" argument definition\n");
        return false;
    }
    a.setDocString(s.c_str());

    bool required = false;
    if (!extractBool(hash, "required", &required)) {
        verboseError.append("'required' specification missing from ");
        verboseError.append(a.name());
        verboseError.append(" argument definition");
        return false;
    }
    a.setRequired(required);

    // now for the type.
    if (!extractString(hash, "type", s)) {
        verboseError.append("'type' missing from ");
        verboseError.append(a.name());
        verboseError.append(" argument definition");
        return false;
    }

    // map string to type
    bp::service::Argument::Type t;
    t = bp::service::Argument::stringAsType(s.c_str());
    if (t == bp::service::Argument::None) {
        verboseError.append("invalid argument type: " + s);
        return false;
    }

    a.setType(t);

    // XXX: need addArgument
    std::list<bp::service::Argument> arguments = f.arguments();
    arguments.push_back(a);
    f.setArguments(arguments);    

    return true;
}


static bool processFunction(VALUE hash, bp::service::Description * desc,
                            std::string & verboseError)
{
    bp::service::Function f;

    std::string s;
    
    if (!extractString(hash, "name", s)) {
        verboseError.append("'name' missing from function definition");
        return false;
    }
    f.setName(s.c_str());

    if (!extractString(hash, "documentation", s)) {
        verboseError.append("'documentation' missing from ");
        verboseError.append(f.name());
        verboseError.append(" function definition");
        return false;
    }
    f.setDocString(s.c_str());

    // now process the arguments
    {
        VALUE rkey = rb_str_new2("arguments");
        VALUE arr = rb_hash_aref(hash, rkey);
        if (rb_type(arr) != T_ARRAY) {
            verboseError.append("'arguments' array missing from definition of");
            verboseError.append(f.name());
            verboseError.append(" function");
            return false;
        }

        for (long int i = 0; true ; i++) {
            VALUE rfHash = rb_ary_entry(arr, i);
            if (rb_type(rfHash) == T_NIL) break;

            if (rb_type(rfHash) != T_HASH) {
                verboseError.append(
                    "non-hash member found in 'arguments' array for function ");
                verboseError.append(f.name());
                return false;
            }

            // now process the function hash
            if (!processArgument(rfHash, f, verboseError)) {
                return false;
            }
        }
    }

    // XXX: need "addFunction"
    std::list<bp::service::Function> l(desc->functions());
    l.push_back(f);
    desc->setFunctions(l);

    return true;
}


static std::string
formatError(const char * e)
{
    std::stringstream ss;
    ss << e << ": " << ruby::getLastError();
    return ss.str();
}


bp::service::Description *
ruby::extractDefinition(std::string& verboseError)
{
    // Global variable "$BrowserPlusEntryPointClass" will reference a 
    // class with a .to_service_description method.  This method returns
    // a stable ruby data structure that we can traverse to build up a
    // c++/c representation of the services interface.
    VALUE defSym = 0;
    {
        VALUE gv = rb_gv_get(BP_GLOBAL_DEF_SYM);
        if (rb_type(gv) != T_CLASS)
        {
            verboseError.append("ruby service lacks entry point class, "
                                "cannot find ");
            verboseError.append(BP_GLOBAL_DEF_SYM);
            verboseError.append(", is bp_doc properly called?");
            return NULL;
        }
        int error = 0;
        defSym = ruby::invokeFunction(gv, BP_EXTERNAL_REP_METHOD, &error, 0);
        if (error) {
            verboseError = formatError("couldn't attain service description");
            return NULL;
        }

        if (rb_type(defSym) != T_HASH) {
            verboseError.append(BP_EXTERNAL_REP_METHOD " returns invalid type");
            return NULL;
        }
    }

    // now we have a HASH ready to traverse!

    bp::service::Description * desc = new bp::service::Description;

    // first grab the name of the service
    std::string s;
    if (!extractString(defSym, "name", s)) {
        verboseError.append("'name' missing from service description");
        delete desc;
        return NULL;
    }
    desc->setName(s.c_str());

    if (!extractString(defSym, "documentation", s)) {
        verboseError.append("'documentation' missing from service description");
        delete desc;        
        return NULL;
    }
    desc->setDocString(s.c_str());

    if (!extractString(defSym, "version", s)) {
        verboseError.append("'version' missing from service description");
        delete desc;        
        return NULL;
    }

    // now parse the version
    {
        bp::ServiceVersion v;
        if (!v.parse(s)) {
            verboseError.append("malformed 'version' string");
            delete desc;        
            return NULL;
        }
        desc->setMajorVersion(v.majorVer());
        desc->setMinorVersion(v.minorVer());        
        desc->setMicroVersion(v.microVer());        
    }
    
    // now process the functions
    {
        VALUE rkey = rb_str_new2("functions");
        VALUE arr = rb_hash_aref(defSym, rkey);
        if (rb_type(arr) != T_ARRAY) {
            verboseError.append("'functions' array missing from service "
                                "description");
            delete desc;
            return NULL;
        }

        for (long int i = 0; true ; i++) {
            VALUE rfHash = rb_ary_entry(arr, i);
            if (rb_type(rfHash) == T_NIL) break;

            if (rb_type(rfHash) != T_HASH) {
                verboseError.append(
                    "non-hash member found in 'functions' array\n");
                delete desc;
                return NULL;
            }

            // now process the function hash
            if (!processFunction(rfHash, desc, verboseError)) {
                delete desc;
                return NULL;
            }
        }
    }

    return desc;
}
