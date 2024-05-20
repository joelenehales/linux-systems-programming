/**
 * Topic:  Scheduling Algorithms
 * Author: Joelene Hales, 2024
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int count_lines(char* filename);
void simulate_FCFS(int** process_info, int num_processes);
void simulate_SJF(int** process_info, int num_processes);
void simulate_RR(int** process_info, int num_processes, int time_quantum);
void run_process(int active, int* burst_time, int* wait_time, int* turnaround_time, int num_processes, int time_elapsed);
int padding(int num);
void print_active_process(int time_elapsed, int process_num, int burst_time, int wait_time, int turnaround_time);
void print_simulation_results(int* process_numbers, int* wait_time, int* turnaround_time, int num_processes);


/**
 * Program to simulate various CPU scheduling algorithms for a set of processes. 
 * 
 * The first command line argument passed to the program specifies which
 * scheduling  algorithm to simulate:
 *   -f : First Come First Served
 *   -s : Shortest Job First
 *   -r : Round Robin
 * A full description of each scheduling algorithm can be found in the function
 * documentation for the corresponding simulation. If Round Robin is given, the
 * next argument to the program is an integer specifying the time quantum. The
 * final argument gives the filepath to a CSV file contaning the process schedule;
 * Each line of the file gives a process number and its burst time, listed in
 * the order they are to be scheduled. An example schedule file is provided in
 * the project directory.
 * 
 * For each unit of time elapsed in the simulation, the program outputs the
 * active process with its remaining burst time and current waiting time and
 * turnaround time. After the simulation, the program lists the final waiting
 * times and turnaround times of each process, and calculates the total
 * averages.
 */
int main(int argc, char * argv[]) {

    /* Unpack filename from input */
    char* filename;
    if (strcmp(argv[1], "-r") == 0) { // Round Robin has an additional parameter (time quantum)
        filename = argv[3];  
    }
    else {
        filename = argv[2];
    }


    /* Count number of lines in file to determine amount of memory to allocate */
    int num_processes = count_lines(filename);


    FILE* file_pointer = fopen(filename, "r");  // Open file

    if (file_pointer == NULL) {  // Check for error opening file
        printf("Unable to open file.");
        exit(0);
    }

    /* Allocate memory for four arrays to store the process number, burst time, *
     * waiting time, and turnaround time respectively. Each element corresponds *
     * to the data for one process.                                             */
    int** process_info = (int**)malloc(4*sizeof(int*));

    for (int i = 0; i < 4; i++) {  // Add an entry for each process in all four arrays
        process_info[i] = (int*)malloc(num_processes*sizeof(int));  
    }


    /* Read process information from CSV file */
    int process_num, burst_time;   // Process number and burst time from file
    int arrival_time = 0;          // Arival time. Equal to the process' index in each array.
    while (fscanf(file_pointer, "P%d,%d\n", &process_num, &burst_time) != EOF) {  // Read until end of file
        
        process_info[0][arrival_time] = process_num;  // Process number
        process_info[1][arrival_time] = burst_time;   // Burst time
        process_info[2][arrival_time] = 0;            // Waiting time
        process_info[3][arrival_time] = 0;            // Turnaround time

        arrival_time++;  // Increment arrival time/array index

    }

    fclose(file_pointer);  // Close file


    /* Run the selected simulation */
    if (strcmp(argv[1], "-f") == 0) {  // First Come First Served
        simulate_FCFS(process_info, num_processes);
    }
    else if (strcmp(argv[1], "-s") == 0) {  // Shortest Job First
        simulate_SJF(process_info, num_processes);
    }
    else if (strcmp(argv[1], "-r") == 0) {  // Round Robin
        simulate_RR(process_info, num_processes, atoi(argv[2]));
    }
    else {
        printf("Invalid scheduling algorithm.");
        exit(0);
    }


    /* List the final waiting time and turnaround time for each process, and calculate the total averages */
    print_simulation_results(process_info[0], process_info[2], process_info[3], num_processes);

    /* Free dynamically allocated memory */
    for (int i = 0; i < 4; i++) {
        free(process_info[i]);
    }
    free(process_info);

    return 0;

}


/**
 * Counts the number of lines in a file
 * 
 * Parameters
 * ----------
 *   filename : Filename
 * 
 * Returns
 * -------
 *   num_lines : Number of lines in the file
 * 
 */
int count_lines(char* filename) {

    FILE* file_pointer = fopen(filename, "r");  // Open file

    if (file_pointer == NULL) {  // Check for error
        printf("Unable to open file.");
        exit(0);
    }
    
    int num_lines = 0;   // Number of lines in the file
    char buffer[20];     // Assumes maximum line length of 20 characters

    /* Count lines in file */
    while(fgets(buffer, sizeof(buffer), file_pointer) != NULL) {  // Read one line each iteration, until end of file
        num_lines++;  // Increment count for each line read
    }

    fclose(file_pointer);  // Close file

    return num_lines;

}


/**
 * Simulates a First Come First Served scheduling algorithm. The simulation runs
 * each process in the order they arrived, allowing each process to run for its
 * full burst duration. The input data array is modified to record the waiting
 * time and turnaround time of each process as the simulation runs.
 * 
 * Parameters
 * ----------
 *   process_info :   An array of four arrays containing the process number, burst 
 *                    time, waiting time, and turnaround time for each process.
 *                    Each element in the arrays corresponds to one process.
 *                    Assumes each process' index in the arrays is equal to its
 *                    arrival time.
 *   num_processes :  Number of processes. Equal to the length of each array.
 */
void simulate_FCFS(int** process_info, int num_processes) {

    printf("First Come First Served\n");  // Scheduling algorithm being simulated

    /* Unpack process information */
    int* process_number = process_info[0];
    int* burst_time = process_info[1];
    int* wait_time = process_info[2];
    int* turnaround_time = process_info[3];

    /* Initialize state of simulation */
    int time_elapsed = 0;  // Time elapsed in the simulation


    /* Run simulation */
    for (int active = 0; active < num_processes; active++) {  // Run processes in the order they arrived

        /* Each iteration represents one unit of time elapsed */
        while (burst_time[active] > 0) {  // Allow each process to run for its full burst duration

            /* Print current state of active process in the simulation */
            print_active_process(time_elapsed, process_number[active], burst_time[active], wait_time[active], turnaround_time[active]);

            /* Run active process for one unit of time, and update all processes */
            run_process(active, burst_time, wait_time, turnaround_time, num_processes, time_elapsed);

            time_elapsed++;  // Increment time elapsed in simulation

        }
    }

}


/**
 * Simulates a Shortest Job First scheduling algorithm. The algorithm searches
 * through the ready processes (have arrived but not finished), and selects the
 * one with the shortest remaining burst time to be run next. If a process with
 * a shorter burst time arrives, the active process is preempted, and a context
 * switch occurs to run the new process. This repeats until all processes have
 * finished (remaining burst time is 0 for all processes). The input data array
 * is modified to record the waiting time and turnaround time of each process as
 * the simulation runs.
 * 
 * Parameters
 * ----------
 *   process_info :   An array of four arrays containing the process number, burst 
 *                    time, waiting time, and turnaround time for each process. Each
 *                    element in the arrays corresponds to one process. Assumes
 *                    each process' index in the arrays is equal to its arrival
 *                    time.
 *   num_processes :  Number of processes. Equal to the length of each array.
 */
void simulate_SJF(int** process_info, int num_processes) {

    printf("Shortest Job First\n");  // Scheduling algorithm being simulated

    /* Unpack process information */
    int* process_number = process_info[0];
    int* burst_time = process_info[1];
    int* wait_time = process_info[2];
    int* turnaround_time = process_info[3];

    /* Initialize state of simulation */
    int selected = 0;            // Index of process to run next (shortest burst time)
    int shortest_burst = 9999;   // Minimum burst time found at a given point in time
    int time_elapsed = 0;        // Time elapsed in the simulation
    int num_completed = 0;       // Number of processes finished (have a remaining burst time of 0)


    /* Run simulation. Each iteration represents one unit of time elapsed */
    while (num_completed < num_processes) {  // Run simulation until all processes have finished

        /* Find process with the shortest burst time at the current time in the simulation */
        for (int arrival_time = 0; arrival_time < num_processes; arrival_time++) {  // Iterate through all processes

            if (burst_time[arrival_time] <= shortest_burst &&  // Process has shortest burst time
                time_elapsed >= arrival_time && burst_time[arrival_time] > 0) {  // Process has arrived and has not finished 

                 selected = arrival_time;                    // Select process to be the active process
                 shortest_burst = burst_time[arrival_time];  // Update minimum burst time found
            
            }
        }

        /* Print current state of active process in the simulation */
        print_active_process(time_elapsed, process_number[selected], burst_time[selected], wait_time[selected], turnaround_time[selected]);

        /* Run selected process for one unit of time, and update all processes */
        run_process(selected, burst_time, wait_time, turnaround_time, num_processes, time_elapsed);
        
       if (burst_time[selected] == 0) {  // Active process has finished
            num_completed++;        // Increment the number of processes completed
            shortest_burst = 9999;  // Reset shortest burst time
       }
        
        time_elapsed++;  // Increment time elapsed in simulation  

    }

}


/**
 * Simulates a Round Robin scheduling algorithm with a given time quantum. The
 * simulation runs each process in the order they arrived. Each process is
 * allowed run for one time quantum, after which it is preempted, and moved to
 * the back of the queue. If a process' remaining burst time is less than the
 * time quantum, it is run until it finishes. This repeats until all processes
 * have finished (remaining burst time is 0 for all processes). The input data
 * array is modified to record the waiting time and turnaround time of each
 * process as the simulation runs.
 * 
 * Parameters
 * ----------
 *   process_info :   An array of four arrays containing the process number, burst 
 *                    time, waiting time, and turnaround time for each process.
 *                    Each element in the arrays corresponds to one process.
 *                    Assumes each process' index in the arrays is equal to its
 *                    arrival time.
 *   num_processes :  Number of processes. Equal to the length of each array.
 *   time_quantum  :  Maximum time each process is allowed to run before being 
 *                    preempted. 
 */
void simulate_RR(int** process_info, int num_processes, int time_quantum) {

    printf("Round Robin with Quantum %d\n", time_quantum);  // Scheduling algorithm being simulated

    /* Unpack process information */
    int* process_number = process_info[0];
    int* burst_time = process_info[1];
    int* wait_time = process_info[2];
    int* turnaround_time = process_info[3];

    /* Initialize state of simulation */
    int time_elapsed = 0;        // Time elapsed in the simulation
    int active = 0;              // Index of current active process
    int remaining_time = 0;      // Time the active process is allowed to run before being preempted
    int num_completed = 0;       // Number of processes finished (have a remaining burst time of 0)
    

    /* Run simulation */
    while (num_completed < num_processes) {  // Run simulation until all processes have finished

        for (int active = 0; active < num_processes; active++) {  // Run processes in the order they arrived

            if (time_elapsed >= active && burst_time[active] > 0) {  // Process has arrived and has not finished 
                
                /* Define period of time for process to run */
                if (burst_time[active] < time_quantum) {  // Remaining burst time shorter than the time quantum
                    remaining_time = burst_time[active];  // Run process until it finishes
                }
                else {
                    remaining_time = time_quantum;  // Run until time quantum expires
                }
                
                /* Allow active process to run for its allotted time */
                while (remaining_time > 0) {  // Each iteration represents one unit of time elapsed
                    
                    /* Print current state of active process in the simulation */
                    print_active_process(time_elapsed, process_number[active], burst_time[active], wait_time[active], turnaround_time[active]);

                    /* Run active process for one unit of time, and update all processes */
                    run_process(active, burst_time, wait_time, turnaround_time, num_processes, time_elapsed);

                    remaining_time--;  // Decrement active process' remaining time to run

                    if (burst_time[active] == 0) {  // Active process has finished
                        num_completed++;  // Increment the number of processes completed
                    }

                    time_elapsed++;  // Increment time elapsed in simulation                

                }

            }

        }
    }

}


/**
 * Updates the state of all processes in the simulation as if the given process
 * was run for one unit of time.
 * 
 * Parameters
 * ----------
 *   active :           Index of process selected to be run.
 *   burst_time :       Current remaining burst times of each process in the simulation
 *   wait_time :        Current waiting times of each process in the simulation.
 *   turnaround_time :  Current turnaround times of each process in the simulation.
 *   num_processes :    Number of processes. Equal to the length of each array.
 *   time_elapsed :     Time elapsed in the simulation
 */
void run_process(int active, int* burst_time, int* wait_time, int* turnaround_time, int num_processes, int time_elapsed) {

    /* Update the state of all processes */
    for (int arrival_time = 0; arrival_time < num_processes; arrival_time++) {  // Iterate over all processes
        
        if (time_elapsed >= arrival_time && burst_time[arrival_time] > 0) {  // Process has arrived and has not finished

            if (arrival_time != active) {  // Process is waiting (arrived but not the active process)
                wait_time[arrival_time] += 1;  // Increment wait time
            }

            turnaround_time[arrival_time] += 1;  // Increment turnaround time                  

        }
    }

    burst_time[active] -= 1;  // Decrement active process' remaining burst time

}


/**
 * Returns the amount of padding to add before or after an integer in formatted
 * output. Assumes that given integer will be positive and a maximum of three
 * digits long for proper formatting.
 * 
 * Parameters
 * ----------
 *   num :  Number to be printed.
 */
int padding(int num) {

    int padding_amount;  // Amount of padding

    if (num < 10) {  // One-digit integer
        padding_amount = 2;
    }
    else if (num < 100) {  // Two-digit integer
        padding_amount = 1;
    }
    else {  // Three-digit integer
        padding_amount = 0;
    }

    return padding_amount;

}


/**
 * Prints the state of the simulation at a given point in time for the active
 * process. Assumes that all values will be a maximum of three digits long for
 * proper formatting.
 * 
 * Parameters
 * ----------
 *   time_elapsed :     Time elapsed in the simulation.
 *   process_num :      Process number of active process
 *   burst_time :       Current remaining burst time of active process
 *   wait_time :        Current wait time of active process
 *   turnaround_time :  Current turnaround time of active process
 */
void print_active_process(int time_elapsed, int process_num, int burst_time, int wait_time, int turnaround_time) {

    /* Print current state of the simulation for the active process */
    printf("T%d%*s : P%d%*s - Burst left %*s%d, Wait time %*s%d, Turnaround time %*s%d\n", time_elapsed, padding(time_elapsed), "", process_num, padding(process_num), "", padding(burst_time), "", burst_time, padding(wait_time), "", wait_time, padding(turnaround_time), "", turnaround_time);

}


/**
 * Lists the final waiting times and turnaround times of each process after the
 * simulation, and calculates the total averages.
 * 
 * Parameters
 * ----------
 *   process_numbers :  Process numbers.
 *   wait_time :        Waiting times of each process after the simulation.
 *   turnaround_time :  Turnaround times of each process after the simulation.
 *   num_processes :    Number of processes. Equal to the length of each array.
 */
void print_simulation_results(int* process_numbers, int* wait_time, int* turnaround_time, int num_processes) {

    /* Initialize totals */
    int total_wait_time = 0;
    int total_turnaround_time = 0;

    for (int i = 0; i < num_processes; i++) {  // Iterate over each process

        /* Print process' wait time and turnaround time */
        printf("\nP%d\n", process_numbers[i]);
        printf("        Waiting time:         %*s%d\n", padding(wait_time[i]), "", wait_time[i]);
        printf("        Turnaround time:      %*s%d\n", padding(turnaround_time[i]), "", turnaround_time[i]);

        /* Add wait time and turnaround time to the running totals */
        total_wait_time += wait_time[i];
        total_turnaround_time += turnaround_time[i];

    }

    /* Calculate total averages and print results */
    printf("\nTotal average waiting time:     %0.1f\n", (float)total_wait_time/num_processes);
    printf("Total average turnaround time:  %0.1f\n", (float)total_turnaround_time/num_processes);

}