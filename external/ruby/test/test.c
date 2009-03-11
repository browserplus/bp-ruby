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
        ruby_script("My ruby embedding test");
        ruby_incpush("../ruby_build_output/lib/ruby/1.9.1");
        ruby_incpush("../ruby_build_output/lib/ruby/1.9.1/i386-darwin9.6.0");
        rb_eval_string_protect("puts \"load path: #{$LOAD_PATH}\"", &error);
        printf("printing loadpath: %d\n", error);
        rb_eval_string_protect("require 'digest/sha2'", &error);
        printf("requiring something that exists: %d\n", error);

        rb_eval_string_protect("require 'digest/sha17'", &error);
        printf("requiring something that doesn't exist: %d\n", error);

        // now do something 

// not found and crash
//        rb_eval_string_protect("require 'digest/sha2'", &error);
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
