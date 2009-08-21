#ifndef __RUBYHEADERS_HH__
#define __RUBYHEADERS_HH__

extern "C" {
    #include <ruby/config.h>
    #include <ruby/ruby.h>
    #include <ruby/intern.h>
    #undef fclose
};

#include <stdio.h>

#endif
