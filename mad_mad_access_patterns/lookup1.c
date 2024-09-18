/**
 * mad_mad_access_patterns
 * CS 341 - Fall 2023
 collabrate with yifan20
 */
#include "tree.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses fseek() and fread() to access the data.

  ./lookup1 <data_file> <word> [<word> ...]
*/

int search(FILE* file, uint32_t offset, char* word) {
    if (offset == 0) {
        return 0;
    }    
    fseek(file, offset, SEEK_SET);
    // move the file position indicator to a specific location in a file.
    BinaryTreeNode new_node;
    fread(&new_node, sizeof(BinaryTreeNode), 1, file);
    // reads a number of items of a specified size from the given file stream into an array.

    fseek(file, sizeof(BinaryTreeNode)+offset, SEEK_SET);
    char temp[10];
    fread(temp, 10, 1, file);

    int result = strcmp(word, temp);
    if (result == 0) {
        printFound(temp, new_node.count, new_node.price);
        return 1;
    } 
    else if (result < 0) {
        return search(file, new_node.left_child, word);
    } 
    else {
        return search(file, new_node.right_child, word);
    }

    return 0;
}

int main(int argc, char **argv) {
    if (argc <= 2) {
        printArgumentUsage();
        exit(1);
    }
    FILE* file = fopen(argv[1], "r");
    if (file == NULL) {
        openFail(argv[1]);
        exit(2);
    }
    char temp[4];
    fread(temp, 1, 4, file);
    if (strcmp(temp, "BTRE")) {
        formatFail(argv[1]);
        exit(2);
    }

    for (int i = 2; i < argc; i++) {
        int find = search(file, 4, argv[i]);
        if (!find)
            printNotFound(argv[i]);
    }
    fclose(file);
    return 0;
}

//I used GPT to reformat my files
