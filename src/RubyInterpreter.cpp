#include "RubyInterpreter.hh"

#include "i386-darwin9.6.0/ruby/config.h"
#include "ruby.h"

static char ** s_argv;
static int s_argc = 1;

void ruby::initialize(const std::string & path)
{
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
    }
}

void ruby::shutdown(void)
{
    (void) ruby_finalize();
}
