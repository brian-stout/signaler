#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <signal.h>
#include <stdbool.h>
#include <sysexits.h> 
#include <math.h>

volatile sig_atomic_t got_sigusr1, got_sigusr2, got_sighup;

void signal_handler(int signal);

bool is_prime(size_t number);

bool argument_checker(int argc, char *argv[]);

int main(int argc, char *argv[])
{
    if (argument_checker(argc, argv)) {
        return EX_USAGE;
    }

    long int counter = 2;
    long int end_number = 0;
    got_sigusr1 = false;
    got_sigusr2 = false;
    got_sighup = false;

    int c;
    bool s_flag = false;
    bool e_flag = false;
    bool r_flag = false;
    while (-1 < (c = getopt(argc, argv, "s:e:r")))
    {
        char *err;


        switch (c)
        {
        case 's':
            if (s_flag == true) {
                printf("Error: Do not use duplicate flags!\n");
                return EX_USAGE;
            }
            else {
                s_flag = true;
            }
            //Grabs the string following s and turns it to a number
            errno = 0;
            counter = strtol(optarg, &err, 10);
            //if strtol fails it's likely because of this error
            if (*err)
            {
                printf("Error: -s must be followed by a valid number \n");
                return EX_USAGE;
            }
            //avoids weird negative numbers as user input
            if (counter < 0)
            {
                printf("Error: -s can not be followed by a negative number \n");
                return EX_USAGE;
            }
            if (errno) {
                printf("An error was detected\n");
                return EX_USAGE;
            }
            break;
        case 'e':
            if (e_flag == true) {
                printf("Error: Do not use duplicate flags!\n");
                return EX_USAGE;
            }
            else {
                e_flag = true;
            }
            //Grabs the string following s and turns it to a number
            errno = 0;
            end_number = strtol(optarg, &err, 10);
            //if strtol fails it's likely because of this error
            if (*err)
            {
                printf("Error: -e must be followed by a valid number \n");
                return EX_USAGE;
            }
            //avoids weird negative numbers as user input
            if (counter < 0)
            {
                printf("Error: -e can not be followed by a negative number \n");
                return EX_USAGE;
            }
            if (errno) {
                printf("An error was detected\n");
                return EX_USAGE;
            }
            break;
        case 'r':
            //Flag for printing out results in reverse
            if (r_flag == true) {
                printf("Error: Do not use duplicate flags!\n");
                return EX_USAGE;
            }
            else {
                r_flag = true;
            }
            got_sigusr2 = true;
            break;
        case '?':
            return EX_USAGE;
        }
    }

    if (got_sigusr2 == true && counter == 2) {
        printf("Error:  A starting point must be specified for reverse counting\n");
        return EX_USAGE;
    }

    struct sigaction sa;

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

    for(;;) {
        if (got_sighup == true) {
            counter = 2;
            got_sighup = false;
        }

        if (end_number != 0 && counter > end_number) {
            break;
        }

        if (is_prime(counter)) {

            if  (got_sigusr1 == true) {
                got_sigusr1 = false;
            }
            else {
                printf("PID: %d\t Prime: %zd\n", getpid(), counter);

                //Checking to see if the next number is going to be less
                //  than two and it's the reverse direction
                if (counter <= 2 && got_sigusr2 == true) {
                    break;
                }
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

    return 0;
}

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

//http://cartera.me/2012/01/10/primality-testing-and-factorization-in-c/
//http://stackoverflow.com/a/26760082
bool is_prime(size_t number)
{
    if (number <= 3 && number > 1) {
        return true;
    }
    else if (number % 2 == 0 || number % 3 == 0) {
        return false;
    }
    else {
        unsigned long int i;
        for (i = 5; i*i <= number; i+=6) {
            if (number % i == 0 || number%(i + 2) == 0) {
                return false;
            }
        }
        return true;
    }
}

bool
argument_checker(int argc, char *argv[])
{
    bool r = false;

    for (int i = 0; i < argc; ++i)
    {
        if (argv[i][0] == '-')
        {
            //Checks to see if a tack option is followed by a space
            if (argv[i][1] == ' ' || argv[i][1] == '\0')
            {
                printf("Usage: [-s <start prime>] [-e <exit prime>] [-r reverse]\n");
                r = true;
            }
            //Checks to see if non option arguments exist before the options
            else if (argv[1][0] != '-')
            {
                printf("Usage: [-s <start prime>] [-e <exit prime>] [-r reverse]\n");
                r = true;
            }
        }
    }

    return r;
}

