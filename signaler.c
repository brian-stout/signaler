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
    if(argument_checker(argc, argv)) {
        return EX_USAGE;
    }

    //The default count starts on 2, the first prime number
    long int counter = 2;
    //If this number is set by the option arguments the program
    //  will stop before reaching any prime number greater than this
    long int end_number = 0;

    //Boolean value used by signal_handler() to let the main loop
    //  know to skip the next prime number
    got_sigusr1 = false;

    //Boolean value used by signal_handler() and the getopt() function to
    //  let the main lop know to decrement the count instead of
    //  incrementing it (going in reverse)
    got_sigusr2 = false;

    //Booealn value used by signal_handler() to let the main loop
    //  know to reset the count to 2
    got_sighup = false;

    int c;
    //Flags are used to make sure the options aren't duplicated
    bool s_flag = false;
    bool e_flag = false;
    bool r_flag = false;
    while(-1 < (c = getopt(argc, argv, "s:e:r")))
    {
        char *err;

        switch(c)
        {
        case 's':
            if(s_flag == true) {
                printf("Error: Do not use duplicate flags!\n");
                return EX_USAGE;
            } else {
                s_flag = true;
            }
            //Grabs the string following s and turns it to a number
            errno = 0;
            counter = strtol(optarg, &err, 10);
            //if strtol fails it's likely because of this error
            if(*err) {
                printf("Error: -s must be followed by a valid number \n");
                return EX_USAGE;
            }
            //avoids weird negative numbers as user input
            if(counter < 0) {
                printf("Error: -s can not be followed by a negative number \n");
                return EX_USAGE;
            }
            //Used to check for int overflows
            if(errno) {
                printf("An error was detected\n");
                return EX_USAGE;
            }
            break;
        case 'e':
            if(e_flag == true) {
                printf("Error: Do not use duplicate flags!\n");
                return EX_USAGE;
            } else {
                e_flag = true;
            }
            //Grabs the string following s and turns it to a number
            errno = 0;
            end_number = strtol(optarg, &err, 10);
            //if strtol fails it's likely because of this error
            if(*err) {
                printf("Error: -e must be followed by a valid number \n");
                return EX_USAGE;
            }
            //avoids weird negative numbers as user input
            if(counter < 0) {
                printf("Error: -e can not be followed by a negative number \n");
                return EX_USAGE;
            }
            //Checking for int overflows
            if(errno) {
                printf("An error was detected\n");
                return EX_USAGE;
            }
            break;
        case 'r':
            //Flag for printing out results in reverse
            if(r_flag == true) {
                printf("Error: Do not use duplicate flags!\n");
                return EX_USAGE;
            } else {
                r_flag = true;
            }
            got_sigusr2 = true;
            break;
        case '?':
            return EX_USAGE;
        }
    }

    //If the program is running in reverse but starts at too
    //  print out an error to avoid running a pointless loop
    if(got_sigusr2 == true && counter == 2) {
        printf("Error:  A starting point must be specified for reverse counting\n");
        return EX_USAGE;
    }

    //Setting up the signal handle settings
    struct sigaction sa;

    sa.sa_handler = signal_handler;
    sa.sa_flags = 0;
    sigemptyset(&sa.sa_mask);

    //Checking for errors via the signals
    if(sigaction(SIGUSR1, &sa, NULL) == -1) {
        perror("Error on SIGUSR1\n");
    }
    if(sigaction(SIGUSR2, &sa, NULL) == -1) {
        perror("Error on SIGUSR2\n");
    }
    if(sigaction(SIGHUP, &sa, NULL) == -1) {
        perror("Error on SIGHUP\n");
    }
    if(sigaction(SIGINT, &sa, NULL) == -1) {
        perror("Error on SIGINT\n");
    }  

    //Main loop here, counting through all numbers checking if they're prime
    for(;;) {

        //If there's a signal hangup start the count back at 2
        if(got_sighup == true) {
            counter = 2;
            //Reset the flag
            got_sighup = false;
        }

        //Stops the program from stopping before an end number if there
        //  is one
        if(end_number != 0 && counter > end_number) {
            break;
        }

        //Checks to see if the counted number is a prime number
        if(is_prime(counter)) {

            //Checks to see if the flag to skip the next prime number is
            //  set, if it is, set the flag to false again and move one
            if(got_sigusr1 == true) {
                got_sigusr1 = false;
            } else {
                //Printing out the prime number with the PID of the process
                printf("PID: %d\t Prime: %zd\n", getpid(), counter);

                //Checking to see if the next number is going to be less
                //  than two and it's the reverse direction
                if(counter <= 2 && got_sigusr2 == true) {
                    break;
                }
                sleep(1);
            }
        }

        //Decrement the counter if the direction is going in reverse
        if(got_sigusr2 == true) {
            counter--;      
        } else {
            //Increment the counter if the direction is going forward 
            counter++;
        }
    }
    return 0;
}

void signal_handler(int signal)
{
    switch(signal) {
        case SIGUSR1 :
            //Sets a flag that lets the program know to skip the next prime number
            got_sigusr1 = true;
            break;
        case SIGUSR2 :
            //Sets a flag to print numbers in reverse if it hasn't already
            //  If it has already it flips it back
            if (got_sigusr2 == true) {
                got_sigusr2 = false;
            } else {
                got_sigusr2 = true;
            }
            break;
        case SIGHUP :
            //Sets a flag that tells the program to reset the count to zero
            got_sighup = true;
            break;
        case SIGINT:
            //Ctrl+C will exit normally
            write(0, "Received SIGINT, exiting program\n", 33);
            exit(0);
        default:
            //Should never happen
            write(0, "Bad Signal", 10);
            return;
    }
}

//http://cartera.me/2012/01/10/primality-testing-and-factorization-in-c/
//http://stackoverflow.com/a/26760082
bool is_prime(size_t number)
{
    //Gets the 2 and 3 edge cases out of the way
    if(number <= 3 && number > 1) {
        return true;
    //2 and 3 are the most common non-prime number products
    } else if(number % 2 == 0 || number % 3 == 0) {
        return false;
    } else {
        //If everything else, brute foce checks
        unsigned long int i;
        //You only have to check up to the square root of a number
        //  because checking if a number is a product checks the other
        //  complimentary product as well
        for(i = 5; i*i <= number; i+=6) {
            if(number % i == 0 || number%(i + 2) == 0) {
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

    for(int i = 0; i < argc; ++i)
    {
        if(argv[i][0] == '-') {
            //Checks to see if a tack option is followed by a space
            if(argv[i][1] == ' ' || argv[i][1] == '\0') {
                printf("Usage: [-s <start prime>] [-e <exit prime>] [-r reverse]\n");
                r = true;
            } else if (argv[1][0] != '-')
            //Checks to see if non option arguments exist before the options
            {
                printf("Usage: [-s <start prime>] [-e <exit prime>] [-r reverse]\n");
                r = true;
            }
        }
    }

    return r;
}

