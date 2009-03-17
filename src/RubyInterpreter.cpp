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
#include "util/fileutils.hh"

#include "RubyInterpreter.hh"
#include "RubyWork.hh"
#include "RubyUtils.hh"

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
static std::list<ruby::Work *> s_workQueue;

static void * rubyThreadFunc(void * ctx)
{
    std::string path((const char *) ctx);

    s_argv = (char **) calloc(2, sizeof(char *));
    s_argv[0] = "BrowserPlus Embedded Ruby";
    s_argv[1] = NULL;

    // still unclear wether this is all right.  probably performed in
    // a thread spawned at allocation time
    ruby_sysinit(&s_argc, &s_argv);
    // XXX: no longer must call NtInitialize on win32??
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
        s_rubyLock.unlock();

        // now we'll block and wait for work
        std::cout << "ruby thread is running" << std::endl;
        
        while (s_running) {
            // pop an item off the queue and process it,
            // outside of the global data structure lock
            ruby::Work * work = NULL;
            
            {
                s_rubyLock.lock();
                if (s_workQueue.size() > 0) {
                    work = *(s_workQueue.begin());
                    s_workQueue.erase(s_workQueue.begin());
                } 
                s_rubyLock.unlock();
            }
                
            if (work == NULL) {
                VALUE sleepyTime = rb_float_new(0.05);
                // run the interpreter for a little bit to let background
                // threads continue
                // XXX: this absolutely sucks.  How do we kick this
                //      ruby thread out of sleep when new work arrives?
                //      signals?  something that we can do from another
                //      thread safely is needed here.
                rb_funcall(rb_mKernel, rb_intern("sleep"), 1, sleepyTime);
            } else {
                if (work->m_type == ruby::Work::T_LoadService) {
                    // first lets update require path
                    std::string serviceDir = file::dirname(work->sarg);
                    ruby_incpush(serviceDir.c_str());
                    std::cout << "updated ruby include path with dir: "
                              << serviceDir << std::endl;

                    // read ruby source file
                    std::string source = file::readFile(work->sarg);

                    int error = 0;
                    VALUE d = rb_eval_string_protect(source.c_str(), &error);
                    if (error) {
                        std::cout << "Error encountered loading service:"
                                  << ruby::getLastError()
                                  << std::endl;
                    } else {
                        // how will we turn "d" into a bp::ServiceDescription?
                    }
                }

                // presence of syncLock indicates synchronous operation
                // (client freed)
                if (work->m_syncLock != NULL) {
                    work->m_syncLock->lock();
                    work->m_done = true;
                    work->m_syncCond->signal();
                    work->m_syncLock->unlock();
                } else {
                    delete work;
                }
            }
        }

        // now we'll block and wait for work
        std::cout << "ruby thread is exiting" << std::endl;
        
        s_rubyLock.unlock();

        // XXX
    }
}

// a utility routine to synchronously run "work" on the ruby interpreter
// thread, blocking until the work is complete
static void runWorkSync(ruby::Work * work)
{
    // *note* in this case work will not be deleted by this function nor
    // the interpreter thread
    s_rubyLock.lock();
    s_workQueue.push_back(work);
    s_rubyCond.signal();
    work->m_syncLock = new bp::sync::Mutex;
    work->m_syncCond = new bp::sync::Condition;
    work->m_syncLock->lock();
    s_rubyLock.unlock();
    do { work->m_syncCond->wait(work->m_syncLock); } while (!work->m_done);
    work->m_syncLock->unlock();
}

// a utility routine to asynchronously run "work" on the ruby interpreter
// thread. work is dynamically allocated by the caller and will
// be freed by the interpreter thread
static void runWorkASync(ruby::Work * work)
{
    s_rubyLock.lock();
    s_workQueue.push_back(work);
    s_rubyCond.signal();
    s_rubyLock.unlock();
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
    ruby::Work work(ruby::Work::T_LoadService);
    work.sarg.append(pathToRubyFile);
    
    runWorkSync(&work);

    

    return NULL;
}
