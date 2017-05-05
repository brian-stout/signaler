#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>

volatile sig_atomic_t got_usr1;

void signal_handler(int signal)
{
    switch(signal) {

    case SIGUSR1 :
        write(0, "SIGUSR1\n", 10);
        break;
    case SIGUSR2 :
        write(0, "SIGUSR2\n", 10);
        break;
    case SIGHUP :
        write(0, "SIGHUP\n", 9);
        break;
    case SIGINT:
        write(0, "Received SIGINT, exiting program\n", 33);
        exit(0);
    default:
        write(0, "Bad Signal", 10);
    }
}

int main(void)
{
    struct sigaction sa;

    got_usr1 = 0;
    sa.sa_handler = signal_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    if (sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("Error on SIGUSR1\n");
    }
    if (sigaction(SIGUSR2, &sa, NULL) == -1) {
        perror("Error on SIGUSR2\n");
    }
    if (sigaction(SIGHUP, &sa, NULL) == -1) {
        perror("Error on SIGHUP\n");
    }
    if (sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error on SIGINT\n");
    }  

    // Change 1 to true later
    while (1) {
        printf("PID %d: working hard...\n", getpid());
        sleep(1);
    }

    printf("Done in by SIGUSR1!\n");

    return 0;
}

