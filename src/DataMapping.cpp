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
 * DataMapping.cpp - tools to map data back and forth from ruby's VALUE to
 *                 bp::Object 
 */

#include "DataMapping.hh"
#include "RubyUtils.hh"
#include "BuiltIns.hh"

static int hashPopulator(VALUE key, VALUE value, VALUE callbackData)
{
    bp::Map * m = (bp::Map *) callbackData;
    m->add(RSTRING_PTR(key), rubyToBPObject(value));
    return ST_CONTINUE;
}

bp::Object *
rubyToBPObject(VALUE v)
{
    bp::Object * obj = NULL;
    
    switch (TYPE(v)) {
        case T_FLOAT:
            obj = new bp::Double(rb_num2dbl(v));
            break;
        case T_STRING:
            obj = new bp::String(RSTRING_PTR(v));
            break;
        case T_FIXNUM:
            obj = new bp::Integer(rb_num2ull(v));
            break;
        case T_TRUE:
            obj = new bp::Bool(true);
            break;
        case T_FALSE:
            obj = new bp::Bool(false);
            break;
        case T_HASH:
            obj = new bp::Map;
            rb_hash_foreach(v,
                            (int (*)(ANYARGS)) hashPopulator,
                            (VALUE) obj);
            break;
        case T_ARRAY:
        {
            long i;
            bp::List * l = new bp::List;
            for (i=0; i < RARRAY_LEN(v); i++) {
                l->append(rubyToBPObject(rb_ary_entry(v, i)));
            }
            obj = l;
        }
        break;
        case T_OBJECT: {
            // map Pathname objects into BPTPath types
            VALUE id = rb_intern("Pathname");
            VALUE klass = 0;
            if (rb_const_defined(rb_cObject, id) &&
                (klass = rb_const_get(rb_cObject, id)) &&
                TYPE(klass) == T_CLASS)
            {
                VALUE r = rb_obj_is_kind_of(v, klass);
                if (RTEST(r)) {
                    // convert to abs path
                    int error = 0;
                    VALUE absPath =
                        ruby::invokeFunction(v, "realpath", &error, 0);
                    VALUE pathString =
                        ruby::invokeFunction(absPath, "to_s", &error, 0);
                    if (!error && TYPE(pathString) == T_STRING) {
                        obj = new bp::Path(RSTRING_PTR(pathString));
                    }
                    break;
                }
            }
        }
        case T_NIL:
        default:
            obj = new bp::Null();
            break;
    }
    
    return obj;
}

VALUE
bpObjectToRuby(const bp::Object * obj,
               unsigned int tid)
{
    if (obj == NULL) return Qnil;

    VALUE v = Qnil;

    switch (obj->type()) {
        case BPTNull:
            v = Qnil;
            break;
        case BPTBoolean:
        {
            if (((bp::Bool *) obj)->value()) v = Qtrue;
            else v = Qfalse;
            break;
        }
        case BPTInteger:
            v = rb_ull2inum(((bp::Integer *) obj)->value());
            break;
        case BPTCallBack: {
            VALUE args[2];
            args[0] = rb_uint2inum(tid);
            args[1] = rb_ull2inum(((bp::Integer *) obj)->value());
            v = rb_class_new_instance(2, args, bp_rb_cCallback);
            break;
        }           
        case BPTDouble:
            v = rb_float_new(((bp::Double *) obj)->value());
            break;
        case BPTString:
            v = rb_str_new2(((bp::String *) obj)->value());
            break;
        case BPTNativePath: {
            VALUE id = rb_intern("Pathname");
            VALUE klass = 0;
            if (rb_const_defined(rb_cObject, id) &&
                (klass = rb_const_get(rb_cObject, id)) &&
                TYPE(klass) == T_CLASS)
            {
                std::string path = ((bp::Path *) obj)->value();
                VALUE p = rb_str_new2(path.c_str());
                v = rb_class_new_instance(1, &p, klass);
            }
            break;
        }
        case BPTMap: 
        {
            bp::Map * m = (bp::Map *) obj;
            v = rb_hash_new();
            bp::Map::Iterator i(*m);
            const char * key;
            while (NULL != (key = i.nextKey())) {
                rb_hash_aset(v,ID2SYM(rb_intern(key)), 
                             bpObjectToRuby(m->value(key), tid));
            }
            
            break;
        }
        
        case BPTList: 
        {
            bp::List * l = (bp::List *) obj;

            v = rb_ary_new();
            
            unsigned int i;
            for (i=0; i < l->size(); i++) {
                rb_ary_push(v, bpObjectToRuby(l->value(i), tid));
            }
            
            break;
        }
        case BPTAny: 
            // invalid
            break;
    }
    
    return v;
}
