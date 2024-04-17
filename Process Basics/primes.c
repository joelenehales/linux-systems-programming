/**
 * Topic:  Process basics.
 * Author: Joelene Hales, 2024
 */

#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

int ceiling(float x);
void countAndSumPrimes(int start, int end);


/**
 * Program to count and sum prime numbers within a given range using basic
 * process concepts.
 * 
 * The program accepts 3 integer command-line parameters. The first parameter is
 * a binary flag to run the program in series or in parallel. The second and
 * third parameters indicate minimum and maximum values of the range respectively. 
 * The range is inclusive of the minimum, but exclusive of the maximum.
 * 
 * The program divides the given range into 4 equally-sized intervals. If run in
 * parallel, the program creates 4 children processes which each count and sum
 * the primes in one interval. If run in series, the computations for all 4
 * intervals are done by the current process.
 */
int main(int argc, char * argv[]) {

    if (argc != 4) {  // Validate input
        printf("Invalid number of arguments recieved.");
        exit(1);
    }

    int pid = getpid();  // Current process' PID
    printf("Process id: %d\n", pid);

    // Define 4 intervals
    int min = atoi(argv[2]);  // Minimum value to begin summing at
    int max = atoi(argv[3]);  // Maximum value to sum up to
    
    float interval = (max - min) / 4.0; // Number of values to be checked by each process


    if (atoi(argv[1]) == 0) {  // Run program in series

        // Count and sum primes in all 4 intervals
        for (int i = 0; i < 4; i++) {
            countAndSumPrimes(ceiling(min + i*interval), min + (i+1)*interval);
        }

    }
    else {  // Run program in parallel
    
        for (int i = 0; i < 4; i++) {  // Iterate over each interval
            
            if (pid > 0) {  // Parent process

                pid = fork();  // Create a child process

                if (pid < 0) {  // Unable to create child process
                    printf("Error creating child process.");
                    exit(1);
                }
                
                if (i == 3) {  // All 4 children processes have been created
                    while (wait(NULL) > 0);   // Wait for all children processes to finish
                }

            }
    
            if (pid == 0) {  // Child process

                // Count and sum primes in one interval
                countAndSumPrimes(ceiling(min + i*interval), min + (i+1)*interval);
                break;  // Prevent the process from computing all other intervals

            }

        }
    }

    return 0;

}


/**
 * Ceiling function. Computes the smallest intger greater than or equal the
 * given value.
 * 
 * Contains code from Filip Roséen - refp's answer in 
 * https://stackoverflow.com/questions/8549365/determining-if-a-float-has-a-fractional-part
 * 
 * Parameters
 * ----------
 *   x : Value to compute the ceiling function of
 * 
 * Returns
 * -------
 *   Smallest integer greater than or equal to x.
 */
int ceiling(float x) {

    int x_int = (int)x;  // Truncate floating point value

    if (x != x_int) {  // Original value not equal to the truncated value, means it has a fractional part
        x_int += 1;  // Round up
    }

    return x_int;

}


/**
 * Count and sum primes starting from the given start value up to but not
 * including the given end value. Displays the result with the parent and child
 * processes' PIDs.
 * 
 * Contains code from https://www.programiz.com/c-programming/examples/prime-number-intervals
 * 
 * Parameters
 * ----------
 *   start :  Start value
 *   end :    End value
 */
void countAndSumPrimes(int start, int end) {

    int count = 0;  // Number of primes
    unsigned long sum = 0;    // Sum of primes

    int current;    // Current number being checked

    if (start <= 1) {  // Skip 0 and 1
        current = 2;
    }
    else {
        current = start; // Start checking numbers from the given start value
    }

    while (current < end) {  // Check all numbers up to but not including end

        int prime = 1;  // Binary flag if a factor has been found (value is not prime)

        // Check for factors of the value that are not 1 or itself
        for (int i = 2; i <= current / 2; i++) {

            if (current % i == 0) {  // Factor has been found
                prime = 0;  // Number is not prime
                break;
            }
        }

        if (prime == 1) {  // All possible factors have been checked, none found - number is prime
            count += 1;      // Incremement count
            sum += current;  // Add to sum
        }

        current += 1;  // Check next number

    }

    // Display results
    printf("pid: %d, ppid %d - ", getpid(), getppid());
    printf("Count and sum of prime numbers between %d and %d are %d and %lu\n", start, end, count, sum);

}