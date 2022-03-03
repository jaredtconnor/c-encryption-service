#include "stdio.h"
#include "stdlib.h"
#include <time.h>

typedef struct keygen
{

    int *keygen_key;
    int key_len;

} keygen_t;

int cap_alpha_ascii = 65;
int min = 0;
int max = 27;

/*
FUNCTION PROTOTYPES
*/
int *gen_key(int);
int random_allowed_char(int[]);
void init_key(keygen_t *, int);

/*
FUNCTION DECLARATIONS
*/

void init_key(keygen_t *key, int key_len)
{
    key->key_len = key_len;
    key->keygen_key = calloc(key_len + 1, sizeof(int *));
}

/*
    CITATION: Random number generator within range
    https://stackoverflow.com/questions/2509679/how-to-generate-a-random-integer-number-from-within-a-range
*/
int random_char(int allowed_ascii[])
{
    int min = 0;
    int max = 27;

    int random_char_ind = (rand() & (max - min + 1)) + min;

    return allowed_ascii[random_char_ind];
}

int main(int argc, char *argv[])
{

    if (argc == 1)
    {
        fprintf(stderr, "Please enter the correct parameters\n");
        return 1;
    }

    srand(time(0));
    char *num = argv[1];
    int cipher_len = atoi(num);
    keygen_t *keygen = malloc(sizeof *keygen);
    init_key(keygen, cipher_len);

    int allowable_ascii[27] = {0};
    allowable_ascii[0] = 32;

    int k = 1; 

    // Setup acceptable range of ASCII values
    for (int i = 1; i < max; i++)
    {
        allowable_ascii[i] = cap_alpha_ascii;
        cap_alpha_ascii++;
        k++;
    }

    // Populate the key
    for (int i = 0; i < keygen->key_len; i++)
    {
        keygen->keygen_key[i] = random_char(allowable_ascii); 
    }

    // Adding new line
    keygen->keygen_key[keygen->key_len + 1] = 10;

    // Printing the array
    for (int i = 0; i < keygen->key_len; i++)
    {
        fprintf(stdout, "%c", keygen->keygen_key[i]);
    }

    free(keygen); 

    return 1;
}