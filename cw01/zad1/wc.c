#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "wc.h"

#define COMMAND_BUFFER 10000

/**
 * Initialize wc_data struct
 *
 * @param max_block_size Maximum number of blocks
 * @return Structure wc_data with initialized data.
 *       When .data is NULL, .max_block_size indicates an error:
 *       - 1 - max_block_size error - Maximum block size must be greater than 0
 */
wc_data wc_init(int max_block_size)
{
    struct wc_data wc = {NULL, 0, 0};

    if(max_block_size <= 0){
        // Set error code
        wc.max_block_size = 1;
        return wc;
    }

    wc.data = (char **)calloc(max_block_size, sizeof(char*));
    wc.max_block_size = max_block_size;
    wc.current_block_size = 0;
    return wc;
}

/**
 * Start wc command and save output to structure
 *
 * @param wc Pointer to wc_data struct
 * @param filename Path to file
 * @return Index where new wc output was saved else error code:
 *          -1 - no space left in the array
 *          -2 - failed to run wc command
 *          -3 - failed to open file with temporary results
 *          -4 - failed to read file
 *          -5 - failed to remove file with temporary results
 */
int wc_start(struct wc_data *wc, char *filename){
    // Check if there is enough space in the array
    if(wc->current_block_size >= wc->max_block_size){
        return -1; // No space left in the array
    }

    // Run wc command and save output to file
    char * command = malloc(COMMAND_BUFFER); // Allocate memory for command
    strcpy(command, "wc ");
    strcat(command, filename);
    strcat(command, " > /tmp/wc.txt");
    int result = system(command);
    free(command);


    if(result != 0){
        return -2; // Failed to run wc command
    }

    // Open file and read data
    FILE *fp = fopen("/tmp/wc.txt", "r");

    if(fp == NULL){
        return -3; // Failed to open file with temporary results
    }

    fseek(fp, 0L, SEEK_END); // Go to the end of the file
    long file_size = ftell(fp); // Get the file size
    fseek(fp, 0, SEEK_SET); // Go to the beginning of the file

    // Allocate memory for the file
    char *file_data = (char *)calloc(file_size, sizeof(char));
    fread(file_data, sizeof(char), file_size, fp); // Read the file

    if(ferror(fp)){
        return -4; // Failed to read file
    }

    fclose(fp); // Close the file

    // Remove file
    result = system("rm -f /tmp/wc.txt");

    if(result != 0){
        return -5; // Failed to remove file with temporary results
    }

    // Save data to wc_data struct
    // Find first null pointer
    int index = 0;
    while(wc->data[index] != NULL && index < wc->max_block_size){
        index++;
    }

    wc->data[index] = file_data;
    wc->current_block_size++;

    return index;
}

/**
 * Get data from wc_data struct at given index
 *
 * @param wc Pointer to wc_data struct
 * @param index Index of data to get
 * @return Pointer to data at given index
 *         NULL when:
 *          - index is out of range
 *          - there is no data at given index
 */
char * wc_get(struct wc_data *wc, int index){
    if(index >= wc->max_block_size){
        return NULL; // Index out of range
    }

    return wc->data[index];
}


/**
 * Free memory allocated for data at given index
 *
 * @param wc Pointer to wc_data struct
 * @param index Index of data to free
 *
 * @return 0 when success, else error code:
 *         1 - index out of range
 *         2 - no data at this index
 */
int wc_free(struct wc_data *wc, int index){
    if (index >= wc->max_block_size) {
        printf("Index out of range \n");
        return 1;
    }

    if (wc->data[index] == NULL) {
        printf("No data at this index \n");
        return 2;
    }

    free(wc->data[index]);
    wc->data[index] = NULL;
    wc->current_block_size--;

    return 0;
}

/**
 * Free memory allocated for wc_data struct
 *
 * @param wc Pointer to wc_data struct
 * @return 0 when success, else error code:
 *       1 - no data to destroy
 */
int wc_destroy(struct wc_data *wc){
    if (wc->data == NULL) {
        return 1;
    }

    for(int i = 0; i < wc->max_block_size; i++){
        free(wc->data[i]);
    }

    free(wc->data);

    return 0;
}
