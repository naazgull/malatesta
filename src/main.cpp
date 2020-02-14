#include <malatesta/malatesta.h>
#include <iostream>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <csignal>
#include <thread>

auto
dispose_sem(int _signal) -> void {
    char _self_exe_name[512] = { 0 };
    if (readlink("/proc/self/exe", _self_exe_name, 511) != 0)
        ;
    key_t _key = ftok(_self_exe_name, 1);
    int _sem = semget(_key, 1, 0777);
    semctl(_sem, 0, IPC_RMID);
    exit(SIGINT);
}

int
main(int argc, char* argv[]) {
    std::signal(SIGINT, dispose_sem);
    std::signal(SIGHUP, dispose_sem);
    std::signal(SIGQUIT, dispose_sem);
    std::signal(SIGTERM, dispose_sem);
    std::signal(SIGKILL, dispose_sem);
    try {
        malatesta::app _app{ argc, argv };
        _app.start();
    }
    catch (malatesta::wrong_parameter_exception& _e) {
        std::cout << malatesta::timestamp() << " " << _e.what() << std::endl << std::flush;
        return 1;
    }
    catch (malatesta::dont_start_exception& _e) {
    }
    return 0;
}
