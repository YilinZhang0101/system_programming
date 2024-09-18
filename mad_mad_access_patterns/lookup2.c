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
#include <sys/mman.h>

/*
  Look up a few nodes in the tree and print the info they contain.
  This version uses mmap to access the data.

  ./lookup2 <data_file> <word> [<word> ...]
*/

BinaryTreeNode *search(char *addr, uint32_t offset, char *word) {
    if (offset == 0) {
        return 0;
    }
    BinaryTreeNode *node = (BinaryTreeNode *)(addr + offset);
    BinaryTreeNode *temp;
    int result = strcmp(word, node->word);
    if (result == 0) {
        return node;
    } 
    else if (result < 0) {
        temp = search(addr, node->left_child, word);
        if (temp != NULL) {
            return temp;
        }
    } 
    else {
        temp = search(addr, node->right_child, word);
        if (temp) {
            return temp;
        }
    }
    return 0;
}

int main(int argc, char **argv) {
    if (argc <= 2) {
        printArgumentUsage();
        exit(1);
    }
    
    FILE *file = fopen(argv[1], "r");
    if (file == NULL) {
        openFail(argv[1]);
        exit(2);
    }
    fseek(file, 0, SEEK_END);

    size_t size = ftell(file);
    // find out the current position of the file pointer in the file stream.
    char *address = mmap(NULL, size, PROT_READ, MAP_PRIVATE, fileno(file), 0);
    if (address == (void *)-1) {
        mmapFail(argv[1]);
        exit(2);
    }

    if (strncmp(address, "BTRE", 4)) {
        formatFail(argv[1]);
        exit(2);
    }

    for (int i = 2; i < argc; i++) {
        BinaryTreeNode *node = search(address, 4, argv[i]);
        if (node)
            printFound(node->word, node->count, node->price);
        else
            printNotFound(argv[i]);
    }
    fclose(file);
    return 0;
}

//i used GPT to reformat my code
