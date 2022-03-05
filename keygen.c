#include "stdio.h"
#include "stdlib.h"
#include <time.h>

int main(int argc, char *argv[])
{

    // Check usage & args
    if (argc < 2)
    {
        fprintf(stderr, "USAGE: %s keylen\n", argv[0]);
        exit(1);
    }

    srand(time(0));
    int keylen = atoi(argv[1]);

    // Populate the key
    for (int i = 0; i < keylen; i++)
    {
        int randnum = rand();
        char randchar = "ABCDEFGHIJKLMNOPQRSTUVWXYZ "[randnum % 27];
        fprintf(stdout, "%c", randchar);
    }

    fprintf(stdout, "\n");

    return 1;
}