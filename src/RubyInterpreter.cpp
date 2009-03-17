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

#include "util/bpthread.hh"
#include "util/bpsync.hh"

#include "RubyInterpreter.hh"
#include "i386-darwin9.6.0/ruby/config.h"
#include "ruby.h"

#include <iostream>

#include <list>

static char ** s_argv;
static int s_argc = 1;

// the thread upon which the ruby interpreter will run
static bp::thread::Thread s_rubyThread;
static bp::sync::Mutex s_rubyLock;
static bp::sync::Condition s_rubyCond;
static bool s_running = false;
static std::list<int> s_workQueue;

static void * rubyThreadFunc(void * ctx)
{
    std::string path((const char *) ctx);

    s_argv = (char **) calloc(2, sizeof(char *));
    s_argv[0] = "BrowserPlus Embedded Ruby";
    s_argv[1] = NULL;

    // still unclear wether this is all right.  probably performed in
    // a thread spawned at allocation time
    ruby_sysinit(&s_argc, &s_argv);
    {
        RUBY_INIT_STACK;
        ruby_init();
        int error = 0; 
        ruby_script("BrowserPlus Embedded Ruby");
        std::string rbPath = path + "/stdlib";
        std::string soPath = path + "/ext";        
        ruby_incpush(rbPath.c_str());
        ruby_incpush(soPath.c_str());

        // let's release the spawning thread
        s_rubyLock.lock();
        s_running = true;
        s_rubyCond.signal();

        // now we'll block and wait for work
        std::cout << "ruby thread is running" << std::endl;
        
        while (s_running) {
            s_rubyCond.wait(&s_rubyLock);
        }

        // now we'll block and wait for work
        std::cout << "ruby thread is exiting" << std::endl;
        
        s_rubyLock.unlock();

        // XXX
    }
}


void ruby::initialize(const std::string & path)
{
    if (!s_running) {
        s_rubyLock.lock();
        if (s_rubyThread.run(rubyThreadFunc, (void *) path.c_str())) {
            while (!s_running) s_rubyCond.wait(&s_rubyLock);
        }
        s_rubyLock.unlock();        
    }
}

void ruby::shutdown(void)
{
    // stop the ruby thread!
    if (s_running) {
        s_rubyLock.lock();
        s_running = false;
        s_rubyCond.signal();        
        s_rubyLock.unlock();
        s_rubyThread.join();
    }
    
    (void) ruby_finalize();
}

bp::service::Description *
ruby::loadRubyService(const std::string & pathToRubyFile,
                      std::string & oError)
{
    oError.append("implement me!");
    return NULL;
}
