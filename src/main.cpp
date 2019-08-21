#include <malatesta/malatesta.h>
#include <iostream>

int
main(int argc, char* argv[]) {
    try {
        malatesta::app _app(argc, argv);
        _app.start();
    }
    catch (malatesta::wrong_parameter_exception& _e) {
        std::cout << _e.what() << std::endl << std::flush;
        return 1;
    }
    return 0;
}
