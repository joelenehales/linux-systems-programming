/**
 * Topic:   Threading basics.
 * Author:  Joelene Hales, 2024
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

void *thread_validate(int* args);

/**
 * Program validates a 9x9 sudoku puzzle solution using basic threading
 * concepts.
 * 
 * The program accepts one command line argument, specifying a file that
 * contains a 9x9 grid of digits, separated by spaces. Example files are
 * provided. For a 9x9 grid, the program uses data parallelism to divide the
 * task of validating the solution between 27 threads, which are each
 * responsible for a single row, column, or subgrid. Once all threads have
 * finished, the program prints whether the solution was valid or invalid.
 */
int main(int argc, char * argv[]) {

    /* Validate input */
    if (argc != 2) {
        printf("Invalid number of arguments recieved.");
        exit(0);
    }


    /* Allocate memory for the sudoku grid as a 2D array of integers */
    int** sudoku_grid = (int**)malloc(9*sizeof(int*));
    for (int row = 0; row < 9; row++) {
        sudoku_grid[row] = (int*)malloc(9*sizeof(int));
    }

    /* Read sudoku solution from file */
    FILE* file_pointer = fopen(argv[1], "r");  // Open file

    int row = 0, col = 0;
    while (row < 9 && col < 9) {

        /* Read a single integer and store in the grid */
        fscanf(file_pointer, " %d", &sudoku_grid[row][col]);  

        if (col == 8) {  // At the end of the row
            row++; // Move to the beginning of the next row
            col = 0;
        }
        else {
            col++;
        }
    }

    fclose(file_pointer);  // Close file


    /* Allocate memory to store the data to be processed by each thread and the thread's number */
    int** thread_data = (int**)malloc(27*sizeof(int*));
    for (int row = 0; row < 27; row++) {
        thread_data[row] = (int*)malloc(10*sizeof(int));
        thread_data[row][9] = row + 1; // Corresponding thread number
    }

    /* Divide data between threads */
    int index = 0;  // Thread's coresponding index in data array

    /* Subgrids (Threads 1-9) */
    int i = 0, j = 0;  // Used to iterate over rows in columns in the sudoku board
    int k = 0;         // Used to iterate over positions in the thread data array
    while (index < 9) {  // Repeat until all 9 subgrids have been added

        for (int row = 3*i; row < 3*(i+1); row++) {  // Iterate over rows in groups of 3

            for (int col = 3*j; col < 3*(j+1); col++) {  // Iterate over columns in groups of 3
                thread_data[index][k] = sudoku_grid[row][col];
                k++;
            }

        }

        j++;      // Move to next group of 3 columns
        index++;  // Add data to next thread's array
        k = 0;    // Begin at first index of the array

        if (index % 3 == 0) {  // Last subgrid in a row
            i++;    // Move to next row of subgrids
            j = 0;  // Restart from beginning of the row
        }

    }

    /* Rows (Threads 10-18) */
    for (int grid_row = 0; grid_row < 9; grid_row++) {

        for (int i = 0; i < 9; i++) {  // Store row data from sudoku grid in data array
            thread_data[index][i] = sudoku_grid[grid_row][i];
        }

        index++;  // Increment row in thread data array

    }

    /* Columns (Threads 19-27) */
    for (int grid_col = 0; grid_col < 9; grid_col++) {

        for (int i = 0; i < 9; i++) {  // Store column data from sudoku grid in data array
            thread_data[index][i] = sudoku_grid[i][grid_col];
        }

        index++;  // Increment row in thread data array

    }
  

    /* Create threads to validate each row/column/subgrid */
    pthread_t threads[27];  // Stores the threads created
    int* valid[27];         // Boolean results returned from each thread
    
    for (int index = 0; index < 27; index++) {
        if (pthread_create(&threads[index], NULL, thread_validate, thread_data[index])) {
            printf("Error creating threads.");
        }
    }

    /* Join threads before continuing */
    for (int i = 0; i < 27; i++) {
        if (pthread_join(threads[i], (void**) &valid[i]) != 0) {
            printf("Error joining threads.");
        }
    }


    /* Determine correctness of the full solution by checking if any
    row/column/subgrid was invalid */
    char* result_str = "a valid"; // String representation of the result
    for (int i = 0; i < 27; i++) {  // Iterate over each result

        if (*valid[i] == 0) {  // Invalid row/column/subgrid was found
            result_str = "an INVALID";
            break;  // Do not continue checking other results

        }
    }

    /* Print the final result */
    printf("\n%s contains %s solution\n", argv[1], result_str);

    /* Free dynamically allocated memory*/
    free(sudoku_grid);
    free(thread_data);
    for (int i = 0; i < 27; i++) {
        free(valid[i]);
    }

    return 0;

}


/**
 * Determines if a single row/column/subgrid of the sudoku grid is valid, and
 * outputs the result. A row/column/subgrid is valid if it contains no repeated
 * digits.
 * 
 * Parameters
 * ----------
 *   args : Array where the first 9 elements are the numbers in the
 *          row/column/subgrid, and the final digit is the thread number.
 * Returns
 * -------
 *   valid : 1 if the row/column/subgrid is valid, 0 if not.
 */
void *thread_validate(int* args) {

    int thread_num = args[9];

    /* Allocate memory to store the result */
    int* valid = malloc(sizeof(int));
    *valid = 1;  // Boolean indicating if the row/column/subgrid is valid

    /* Check for repeated digits in the given array */ 
    for (int i = 0; i < 9; i++) {  // Iterate over each value in the array

        for (int j = 0; j < 9; j++) {  // Compare value to each other value in the array

            if (i != j && args[i]==args[j]) {  // Same value found in another position
                *valid = 0;  // Row/column/subgrid is invalid
                break;
            }
        }
    }

    /* Determine which region of the sudoku grid was validated based on thread number */
    char* type;   // Whether the region is a row, column, or subgrid
    int num;      // Row/column/subgrid number
    if (thread_num <= 9) {
        type = "subgrid";
        num = thread_num;
    }
    else if (thread_num <= 18) {
        type = "row";
        num = thread_num - 9;
    }
    else {
        type = "column";
        num = thread_num - 18;
    }

    /* Define string to print depending on result */
    char* result_str;  // String representation of the result
    if (*valid == 1) {
        result_str = "valid";
    }
    else {
        result_str = "INVALID";
    }

    /* Add padding to single-digit thread numbers */
    int padding = 0;
    if (thread_num <= 9) {
        padding = 1;
    }

    /* Print result */
    printf("Thread # %*s%d (%s %d) is %s\n", padding, "", thread_num, type, num, result_str);

    return (void *) valid;

}