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
 * An in memory representation of "work" that the interpreter must
 * perform.  This data structure is marshalled to the thread upon
 * which the interpreter is running
 */
#ifndef __RUBY_WORK_HH__
#define __RUBY_WORK_HH__

#include "util/bpsync.hh"

#include "bpservicedescription.hh"

#include "i386-darwin9.6.0/ruby/config.h"
#include "ruby.h"

#include <string>

namespace ruby 
{
    class Work 
    {
    public:
        /** the types of work that can be performed over on the ruby
         *  thread */
        typedef enum {
            /** load a service.  sarg contains the path to the 
             *  service */
            T_LoadService,
            /** Allocate an instance of a service */
            T_AllocateInstance,
            /** Invoke a method on the ruby instance */
            T_InvokeMethod,
            /** Explicitly release and destroy a service */
            T_ReleaseInstance
        } Type;

        Work(Type t);
        ~Work();
        
        Type m_type;
        std::string sarg;

        // a means of returning errors from ruby interpreter thread
        // for synchronous operations
        bool m_error;
        std::string m_verboseError;

        // used during LoadService work to return a service description
        bp::service::Description * m_desc;

        // a means of getting data back and forth
        const bp::Object * m_obj;

        // how the instance object is passed back and forth
        VALUE m_instance;

        // A transaction id, used in method invocation
        unsigned int m_tid;

        // sync primitives to support synchronous work execution,
        // when non-null, these primitives indicate that the work
        // is to be performed synchronously and the caller will free
        // the work memory
        bp::sync::Mutex * m_syncLock;
        bp::sync::Condition * m_syncCond;
        // handle spurious wakeups (I love you, POSIX)
        bool m_done;
    };
};

#endif
