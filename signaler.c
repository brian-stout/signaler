#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h> 

volatile sig_atomic_t got_sigusr1, got_sigusr2, got_sighup;

void signal_handler(int signal)
{
    switch(signal) {

    case SIGUSR1 :
        write(0, "SIGUSR1\n", 10);
        got_sigusr1 = true;
        break;
    case SIGUSR2 :
        write(0, "SIGUSR2\n", 10);
        if (got_sigusr2 == true) {
            got_sigusr2 = false;
        }
        else {
            got_sigusr2 = true;
        }
        break;
    case SIGHUP :
        write(0, "SIGHUP\n", 9);
        got_sighup = true;
        break;
    case SIGINT:
        write(0, "Received SIGINT, exiting program\n", 33);
        exit(0);
    default:
        write(0, "Bad Signal", 10);
        return;
    }
}

bool is_prime(size_t number)
{
    printf("%zd\n", number);
    return true;
}



int main(void)
{
    struct sigaction sa;

    got_sigusr1 = false;
    got_sigusr2 = false;
    got_sighup = false;

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

    size_t counter = 1;
    for(;;) {

        if (got_sighup == true) {
            counter = 2;
            got_sighup = false;
        }

        if (is_prime(counter)) {

            if  (got_sigusr1 == true) {
                got_sigusr1 = false;
            }
            else {
                printf("PID: %d\t Prime: %zd\n", getpid(), counter);
                sleep(1);
            }
        }

        if (got_sigusr2 == true) {
            counter--;      
        }
        else {
            counter++;
        }
    }

    printf("Done in by SIGUSR1!\n");

    return 0;
}

