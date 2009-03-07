#include "ruby-1.9.1/ruby.h"
#include <assert.h>

RUBY_GLOBAL_SETUP

static void runInterpreter(int argc, char ** argv)
{
    ruby_sysinit(&argc, &argv);
    {
        RUBY_INIT_STACK;
        ruby_init();

        int error = 0; 
        // now do something 
        ruby_incpush("../ruby_build_output/lib/ruby/1.9.1");
// not found and crash
        rb_eval_string_protect("require 'digest/sha2'", &error);
        rb_eval_string_protect("foo = \"world\"\nputs \"hi #{foo}\"", &error); 
        printf("evaluated shtuff: %d\n", error); 
        rb_eval_string_protect("foo = \"world\"\nputs \"hello #{foo}\"",
                               &error); 
        printf("evaluated shtuff: %d\n", error); 
    } 
    (void) ruby_finalize();
}


int
main(int argc, char ** argv)
{

#ifdef WIN32
    NtInitialize(&argc, (char ***) &argv);
#endif
    runInterpreter(argc, argv); 

    return 0;
}
