#include "ruby-1.9.1/ruby.h"
#include <assert.h>

// load compiled in extensions.
//extern "C" {
    extern void Init_ext _((void));
//};
void RUBY_load_builtins(void)
{
    Init_ext();
}

//
static void runInterpreter(int argc, char ** argv)
{
    ruby_init();
    ruby_options(argc, argv);
    ruby_script(argv[0]);


    RUBY_load_builtins(); 

    ruby_incpush("../ruby_build_output/lib/ruby/1.9.1");
    rb_require("digest/md5");

    int error = 0;
    // now do something
    printf("evaluating shtuff\n");
    rb_eval_string("foo = 'world'");
    rb_eval_string_protect("puts \"hello #{foo}\"", &error);
    printf("evaluated shtuff: %d\n", error);
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
