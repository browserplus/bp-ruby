#ifndef __RUBYINTERPRETER_HH__
#define __RUBYINTERPRETER_HH__

#include <string>

namespace ruby {
    // intialize the ruby interpreter, given the path to this service.
    // this will call all required initialization routines and
    // will correctly populate load paths.
    void initialize(const std::string & path);

    // shutdown the ruby interpreter, freeing all possible resources.
    void shutdown(void);
}

#endif
