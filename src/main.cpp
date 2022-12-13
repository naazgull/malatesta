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
    return 0;
}
