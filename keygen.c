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
void print_key(keygen_t * keygen);

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
int random_char()
{

    int allowable_ascii[27] = {'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J',
        'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', ' '};

    int min = 0;
    int max = 27;

    int random_char_ind = (rand() & (max - min + 1)) + min;
    return allowable_ascii[random_char_ind];
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

    // Populate the key
    for (int i = 0; i < keygen->key_len; i++)
    {
        keygen->keygen_key[i] = random_char(); 
    }

    keygen->keygen_key[keygen->key_len] = '\0';

    print_key(keygen);
    free(keygen); 

    return 1;
}


void print_key(keygen_t * keygen){ 

    fflush(stdout);
    for (int i = 0; i <= keygen->key_len; i++)
    {
        fprintf(stdout, "%c", keygen->keygen_key[i]);
    }

    return;
}