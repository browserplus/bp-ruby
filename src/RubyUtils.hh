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

#ifndef __RUBYUTILS_HH__
#define __RUBYUTILS_HH__

#include <string>

#include "RubyHeaders.hh"

namespace ruby {
    /** get the last error encountered from the ruby evaluation environment */
    std::string getLastError();

    /** simplified invocation of a function upon a reciever while catching
     *  errors */
    VALUE invokeFunction(VALUE r, const char * funcName, int * error,
                         int nargs, ...);

    // This little class is taken from
    // http://metaeditor.sourceforge.net/embed/
    // The idea is simple, we need anonymous values returned from
    // ruby to have a non zero reference count so that they are not
    // garbage collected.  One option is to interact with the rb_gc_* code
    // directly.  This option is better, cause it's faster and simpler.
    // We allocate an array on the stack, and add elements to this array,
    // because the array is marked, those items will not be garbage collected
    // as long as they're in the array.
    class GCArray {
    public:
        GCArray() {
            objects = rb_ary_new();
            rb_gc_register_address(&objects);
        }
        ~GCArray() {
            // dispose array and flush all elements
            rb_gc_unregister_address(&objects);
        }
        void Register(VALUE object) {
            rb_ary_push(objects, object);
        }
        void Unregister(VALUE object) {
            rb_ary_delete(objects, object);
        }
    private:
        VALUE objects;
    };

};

#endif
