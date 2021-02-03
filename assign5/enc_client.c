#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>  // ssize_t
#include <sys/socket.h> // send(),recv()
#include <netdb.h>      // gethostbyname()

// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(2);
}

// Set up the address struct
void setupAddressStruct(struct sockaddr_in* address, int portNumber){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address));

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);

  // Get the DNS entry for this host name
  struct hostent* hostInfo = gethostbyname("localhost");
  if (hostInfo == NULL) {
    fprintf(stderr, "CLIENT: ERROR, no such host\n");
    exit(2);
  }
  // Copy the first IP address from the DNS entry to sin_addr.s_addr
  memcpy((char*) &address->sin_addr.s_addr, hostInfo->h_addr_list[0], hostInfo->h_length);
}

/* This function  is responsible for returning the contents of a file in the form of a string
 * This function takes in a parameter which is the filename to extract from
 * Returns the string contetnts of the file
 */
char* fileParsed(char* fileName) {
  FILE* mFile = fopen(fileName, "r");//creates and opens the file with the desire only to read
  char* buf;
  char* curLine;
  size_t len = 0;
  ssize_t nread;

  nread = getline(&curLine, &len, mFile);//loop iterates over the file and puts each line into the string curLine
  buf = calloc(strlen(curLine), sizeof(char));//allocates memory for the string to be returned
  strcpy(buf, curLine);
  buf[strlen(buf)-1] = '\0';//makes sure that it is null terminated

  fclose(mFile);
  return buf;
}

/* This function checks the contents of the inputs which is the key and the message
 * Takes in the two values to check (Message and key)
 * Doesnt return anything but quits and prints error if invalid input is given
 * Also checks to make sure that the key is as long as the message 
 */
void inputCheck(char* message, char* key) {
  for(int i = 0; i < strlen(message); i++) {
    if((message[i] < 65 || message[i] > 90) && message[i] != 32) {//Checks in range of upper case chars(65-90) with exceptoin of the space (32)
      fprintf(stderr, "Error, invalid charachters in message\n");
      exit(1);
    }
  }
  
  for(int i = 0; i < strlen(key); i++) {
    if((key[i] < 65 || key[i] > 90) && key[i] != 32) {//Checks same for the key
      fprintf(stderr, "Error, invalid charachters in key\n");
      exit(1);
    }
  }

  if(strlen(key) < strlen(message)) {//Checks if the key is shorter than the message
    fprintf(stderr, "Error, key is too short\n");
    exit(1);
  }
}

int main(int argc, char *argv[]) {
  int socketFD, portNumber, charsWritten, charsRead, sendLen;
  struct sockaddr_in serverAddress;
  char buffer[1000], completeMessage[200000];
  char* messageKey;
  char* message;
  char* key;
  int mesLen;
  int port;

  // Check usage & args
  if (argc < 4) {
    fprintf(stderr,"USAGE: %s hostname port\n", argv[0]);
    exit(2);
  }
  port = atoi(argv[3]);//Need the port early to set up the socket

  // Create a socket
  socketFD = socket(AF_INET, SOCK_STREAM, 0);
  if (socketFD < 0){
    error("CLIENT: ERROR opening socket");
  }

   // Set up the server address struct
  setupAddressStruct(&serverAddress, port);

  // Connect to server
  if (connect(socketFD, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
    fprintf(stderr, "Error, problems connecting to the server on port %d\n", port);
    exit(2);
  }
  
  //Parse all of the information provided on the command line
  message = fileParsed(argv[1]);
  key = fileParsed(argv[2]);

  inputCheck(message, key);//Check input given
  
  mesLen = strlen(message)+ strlen(key) + 4;//Add spaces for '_' seperator, the newline charachter, and the two terminating symbols $$
  messageKey = calloc(mesLen, sizeof(char));//Allocates memory for it
  snprintf(messageKey, mesLen, "%s_%s$$", message, key);//Combines it to one message to send

  char* connectionCheck = "enc_client";//Tells it who is communicating with it
  sendLen = strlen(connectionCheck);
  while((charsWritten = send(socketFD, connectionCheck, sendLen, 0)) < sendLen) {//Sends its name to make sure its talking to the right server
    if(charsWritten < 0) {
      error("ERROR writing to socket");
    }
    connectionCheck = connectionCheck + charsWritten;//Loop incraments till everything is sent
    sendLen = sendLen - charsWritten;
  }
  
  memset(buffer, '\0', sizeof(buffer));
  // Read data from the socket, leaving \0 at end
  charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);//The receive is not set on a loop since only the first charachter is used in the check
  if (charsRead < 0){
    error("CLIENT: ERROR reading from socket");
  }
  if(buffer[0] == 'd') {//If its talking with the dec_server then the program quits and sets its propper error value
    fprintf(stderr, "Cannot connect to this server: dec_server\n");
    exit(2);
  }

  sendLen = strlen(messageKey);
  // Send a Success message back to the client
  while((charsWritten = send(socketFD, messageKey, sendLen, 0)) < sendLen) {//Now send the message to sncrypt and the key which also has a terminating symbol
    if (charsWritten < 0){
      error("ERROR writing to socket");
    }
    messageKey = messageKey + charsWritten;//Lopps until everything is sent
    sendLen = sendLen - charsWritten;
  }

  // Get return message from server
  // Clear out the buffer again for reuse
  memset(completeMessage, '\0', sizeof(completeMessage));
  while(strstr(completeMessage, "@@") == NULL) {//Need entire message so dont stop receiving until the terminating symbol is found
    memset(buffer, '\0', sizeof(buffer));
    charsWritten = recv(socketFD, buffer, sizeof(buffer)-1, 0);
    strcat(completeMessage, buffer);//Adds it to the complete buffer
    if(charsWritten < 0) {
      error("ERROR reading from socket\n");
    }
  }

  //Gets ride of the termiting symbol by replacing its start with a null terminator
  int terminalLocation = strstr(completeMessage, "@@") - completeMessage;
  completeMessage[terminalLocation] = '\0';
  printf("%s\n", completeMessage);//Prints with its newline

  // Close the socket
  close(socketFD);
  return 0;
}
