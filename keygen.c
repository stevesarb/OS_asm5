#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int main(int argc, char *argv[]) {
    // check usage and args
    if (argc < 2) {perror("No length specified\n"); exit(1);}

    int len = atoi(argv[1]), randNum;
    char* key = calloc(len, sizeof(char));

    srand(time(0));
    for (int i = 0; i < len; i++) {
        randNum = rand() % 27 + 65; // returns a value between 65-91

        if (randNum == 91) // space
            key[i] = ' ';
        else
            key[i] = randNum; // uses ascii values to interpret ints as chars
    }

    printf("%s\n", key);
    
    free(key);

    return 0;
}