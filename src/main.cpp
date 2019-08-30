#include <malatesta/malatesta.h>
#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <csignal>

auto
dispose_sem(int _signal) -> void {
    char _buffer[512] = { 0 };
    if (readlink("/proc/self/exe", _buffer, 511) != 0)
        ;
    key_t _key = ftok(_buffer, 1);
    int _sem = semget(_key, 1, 0777 | IPC_CREAT);
    semctl(_sem, 0, IPC_RMID);
    exit(SIGINT);
}

int
main(int argc, char* argv[]) {
    std::signal(SIGINT, dispose_sem);
    try {
        malatesta::app _app(argc, argv);
        _app.start();
    }
    catch (malatesta::wrong_parameter_exception& _e) {
        std::cout << _e.what() << std::endl << std::flush;
        return 1;
    }
    catch (malatesta::dont_start_exception& _e) {
    }
    return 0;
}
