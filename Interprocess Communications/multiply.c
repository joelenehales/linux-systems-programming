/**
 * Topic:  Interprocess communications
 * Author: Joelene Hales, 2024
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

void print_variable(char var);
void send_data(int* port, int write_end, int data, int fork_pid);
int recieve_data(int* port, int read_end, int fork_pid);


/**
 * Program computes the product of two integers using decomposition.
 * 
 * The program accepts two, four-digit integers as command line arguments.
 * The program begins by partitioning each given integer into two, 2-digit
 * components. The parent process forks a child process which is responsible
 * for computing the products of all possible pairs. The parent and child
 * processes pass operands and products between eachother using a bidirectional
 * pipe. A message is printed each time data is sent or recieved. After each product
 * is computed, the parent process computes each required intermediate value.
 * Finally, the parent process sums together the intermediate values to obtain
 * the final result.
 */
int main(int argc, char * argv[]) {

    int a, b, a1, a2, b1, b2;  // Integer to multiply and their components

    /* Validate input */
    if (argc != 3) {
        printf("Invalid number of arguments recieved.");
        exit(0);
    }

    /* Convert input to integers */
    a = atoi(argv[1]);
    b = atoi(argv[2]);
    printf("Your integers are %d %d\n", a, b);

    /* Partition each integer into two components */
    a1 = a / 100;
    a2 = a % 100;
    b1 = b / 100;
    b2 = b % 100; 

    /* Establish a bidirectional pipe */
    int parent_to_child[2];   // Parent writes, child reads
    int child_to_parent[2];   // Child writes, parent reads
    if (pipe(parent_to_child) < 0 || pipe(child_to_parent) < 0) {  // Check for failure
        printf("Error creating pipe.");
        exit(0);
    }
    
    /* Fork a child process */
    int pid = fork();
    if (pid < 0) {  // Check for failure
        printf("Error forking child process.");
        exit(0);
    }


    /* Compute product of integers using decomposition */
    if (pid > 0) {  // Parent process

        printf("Parent (PID %d): created child (PID %d)\n", getpid(), pid);

        int X, Y, Z;  // Intermediate values calculated by the parent process
        int A, B, C;  // Products calculated by the child process
        int result;   // Final product of the given integers
        
        /* Calculate intermediate value X */
        print_variable('X');  // Print message to indicate X is being calculated

        send_data(parent_to_child, 1, a1, pid);  // Send a1, b1 to child process to compute product
        send_data(parent_to_child, 1, b1, pid);

        A = recieve_data(child_to_parent, 0, pid);  // Recieve product sent from child process
        
        X = A * 10000;


        /* Calculate intermediate value Y */
        print_variable('Y');  // Print message to indicate Y is being calculated

        send_data(parent_to_child, 1, a1, pid);  // Send a1, b2 to child process to compute product
        send_data(parent_to_child, 1, b2, pid);
        
        B = recieve_data(child_to_parent, 0, pid);  // Recieve product sent from child process

        send_data(parent_to_child, 1, a2, pid);  // Send a2, b1 to child process to compute product
        send_data(parent_to_child, 1, b1, pid);

        C = recieve_data(child_to_parent, 0, pid);  // Recieve product sent from child process

        Y = (B + C) * 100;


        /* Calculate intermediate value Z */
        print_variable('Z');  // Print message to indicate Z is being calculated

        send_data(parent_to_child, 1, a2, pid);  // Send a2, b2 to child process to compute product
        send_data(parent_to_child, 1, b2, pid);

        Z = recieve_data(child_to_parent, 0, pid);  // Recieve product sent from child process


        wait(NULL);  // Wait for child process to finish computing all products

        /* Sum intermediate values to obtain the final result */
        result = X + Y + Z;
        printf("\n%d*%d == %d + %d + %d == %d\n", a, b, X, Y, Z, result);


    }
    else {  // Child process

        int x, y;      // Operands recieved from parent process
        int product;   // Product of operands recieved
        int done = 0;  // Number of products computed

        while (done < 4) {  // Repeat until all 4 products have been computed

            x = recieve_data(parent_to_child, 0, pid);  // Recieve integers sent from parent process
            y = recieve_data(parent_to_child, 0, pid);

            /* Compute product of the recieved integers */
            product = x * y;
            send_data(child_to_parent, 1, product, pid);  // Send computed product back to parent

            done += 1;  // Increment the number of products computed

        }

    }

    return 0;
}


/**
 * Prints the message indicating which variable is being calculated in the
 * required format.
 * 
 * Parameters
 * ----------
 *   var : Variable being calculated
 **/
void print_variable(char var) {
    printf("\n###\n");
    printf("# Calculating %c\n", var);
    printf("###\n");
}


/**
 * Sends an integer value through a pipe.
 * 
 * Parameters
 * ----------
 *   port :       Pipe used for the process to send data to another process
 *   write_end :  Write end of the pipe (0 or 1)
 *   data :       Integer value to be sent
 *   fork_pid :   PID from forking the child process. Used to print the
 *                appropriate message.
 */
void send_data(int* port, int write_end, int data, int fork_pid) {
    
    /* Print message that data is being sent */
    if (fork_pid > 0) {  // Parent process
        printf("Parent (PID %d): Sending %d to child\n", getpid(), data);
    }
    else {  // Child process
        printf("        Child (PID %d): Sending %d to parent\n", getpid(), data);
    }

    /* Send data through pipe */
    write(port[write_end], &data, 4);

}


/**
 * Recieves an integer value from a pipe.
 * 
 * Parameters
 * ----------
 *   port :      Pipe used for the process to recieve data from another process
 *   read_end :  Read end of the pipe (0 or 1)
 *   fork_pid :  PID from forking the child process. Used to print the
 *               appropriate message.
 * 
 * Returns
 * -------
 *   data : The recieved integer
 */
int recieve_data(int* port, int read_end, int fork_pid) {

    /* Read data from the pipe */
    int data;
    read(port[read_end], &data, 4);

    /* Print message that data has been recieved */
    if (fork_pid > 0) {  // Parent process
        printf("Parent (PID %d): Received %d from child\n", getpid(), data);
    }
    else {  // Child process
        printf("        Child (PID %d): Received %d from parent\n", getpid(), data);
    }
    
    return data;

}

