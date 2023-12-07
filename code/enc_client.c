#include <netdb.h>
#include <fcntl.h>
#include "universal.h"
#include "client.h"

int main(int argc, char *argv[])
{
    int socketFD, portNumber, charsWritten, charsRead;
    struct sockaddr_in serverAddress;
    struct hostent* serverHostInfo;
    char buffer[256];
    char* cipherText = NULL;

    if (argc < 4) { fprintf(stderr,"USAGE: %s hostname port\n", argv[0]); exit(0); } // Check usage & args

    // check text and key files for any bad character and for valid lengths
    char* key = NULL;
    FILE* keyFile = fopen(argv[2], "r"); // open file
    if (keyFile == NULL) error("enc_client: Couldn't open key file\n");
    int keyValid = check_file(keyFile, &key); fclose(keyFile);
    // printf("key: %s\tkeyValid: %d\n", key, keyValid);

    char* msg = NULL;
    FILE* msgFile = fopen(argv[1], "r");
    if (msgFile == NULL) error("enc_client: Couldn't open msg file\n"); //{perror("Couldn't open msg file\n"); exit(1);}
    int msgValid = check_file(msgFile, &msg); fclose(msgFile);
    // printf("msg: %s\tmsgValid: %d\n", msg, msgValid); exit(0);

    if (msgValid < 0) fprintf(stderr, "enc_client: %s has invalid characters\n", argv[1]);
    if (keyValid < 0) fprintf(stderr, "enc_client: %s has invalid characters\n", argv[2]);
    if (strlen(key) < strlen(msg)) error("enc_client: Key is shorter than the message!\n");


    // Set up the server address struct
    memset((char*)&serverAddress, '\0', sizeof(serverAddress)); // Clear out the address struct
    portNumber = atoi(argv[3]); // Get the port number, convert to an integer from a string
    serverAddress.sin_family = AF_INET; // Create a network-capable socket
    serverAddress.sin_port = htons(portNumber); // Store the port number
    serverHostInfo = gethostbyname("localhost"); // Convert the machine name into a special form of address
    if (serverHostInfo == NULL) { fprintf(stderr, "CLIENT: ERROR, no such host\n"); exit(0); }
    memcpy((char*)&serverAddress.sin_addr.s_addr, (char*)serverHostInfo->h_addr, serverHostInfo->h_length); // Copy in the address

    // Set up the socket
    socketFD = socket(AF_INET, SOCK_STREAM, 0); // Create the socket
    if (socketFD < 0) error("CLIENT: ERROR opening socket");
    
    // Connect to server
    if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0) // Connect socket to addy
        error("CLIENT: ERROR connecting");

    
    // check that the client is connected to the encryption server
    check_server(socketFD, 'e');
    
    // Send message to server
    charsWritten = send(socketFD, msg, strlen(msg), 0); // Write to the server
    if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
    if (charsWritten < strlen(msg)) perror("CLIENT: WARNING: Not all data written to socket!\n");

    // send key to server
    charsWritten = send(socketFD, key, strlen(key), 0);
    if (charsWritten < 0) error("CLIENT: ERROR writing to socket");
    if (charsWritten < strlen(key)) perror("CLIENT: WARNING: Not all data written to socket!\n");
    
    // Get return message from server
    cipherText = get_data(socketFD);
    printf("%s\n", cipherText);
    
    close(socketFD); // Close the socket
    return 0;
}