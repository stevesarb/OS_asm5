#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* encrypt_msg(char* msg, char* key) {
    if (strlen(key) < strlen(msg)) error("SERVER: key is too short!\n");

    char* cipherText = calloc(strlen(msg) + 1, sizeof(char));
    memset(cipherText, '\0', strlen(msg) + 1);

    int i = 0, msgCharVal, keyCharVal;
    while (i < strlen(msg)) {
        if (msg[i] == ' ')
            msgCharVal = msg[i] - 6;
        else
            msgCharVal = msg[i] - 65;

        if (key[i] == ' ')
            keyCharVal = key[i] - 6;
        else
            keyCharVal = key[i] - 65;
        
        cipherText[i] = ((msgCharVal + keyCharVal) % 27) + 65;
        if (cipherText[i] == '[')
            cipherText[i] = ' ';

        // printf("msg: %c:%d + key: %c:%d = cipher: %c:%d\n", msg[i], msgCharVal, key[i], keyCharVal, cipherText[i], cipherText[i]) - 65;

        ++i;
    }

    return cipherText;
}

char* decrypt_msg(char* msg, char* key) {
    if (strlen(key) < strlen(msg)) error("SERVER: key is too short!\n");

    char* plainText = calloc(strlen(msg) + 2, sizeof(char));
    memset(plainText, '\0', strlen(msg) + 2);

    int i = 0, msgCharVal, keyCharVal, diff;
    while (i < strlen(msg)) { // loop through msg string
        if (msg[i] == ' ')
            msgCharVal = msg[i] - 6; // treat spaces as a 27th character (numeric value of 26)
        else
            msgCharVal = msg[i] - 65; // assign all other characters a numeric value 0(A)-25(Z)

        // repeat for key
        if (key[i] == ' ')
            keyCharVal = key[i] - 6;
        else
            keyCharVal = key[i] - 65;
        
        diff = msgCharVal - keyCharVal;
        if (diff < 0)
            diff += 27;
        plainText[i] = (diff % 27) + 65; // decryption

        // decryption doesn't handle spaces properly, so we must assign them manually
        if (plainText[i] == '[')
            plainText[i] = ' ';

        ++i;
    }

    plainText[i] = '!'; // add control code to end of string

    return plainText;
}

int main() {
    char* msg = "HELLO WORLD MY NAME IS STEVEN";
    char* key = "DESUO POKIVFPXSDSLWJGMWEETDLLZZPDQI";
    char* cipherText = encrypt_msg(msg, key);
    printf("cipherText: %s\n", cipherText);
    char* plainText = decrypt_msg(cipherText, key);
    printf("plaintext: %s\n", plainText);

    return 0;
}