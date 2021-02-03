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

/* Exact Same functionality as enc_client version
 *
 */
char* fileParsed(char* fileName) {
  FILE* mFile = fopen(fileName, "r");//creates and opens the file with the desire only to read
  char* buf; //Creates this variable
  char* curLine;//Gets in the input line
  size_t len = 0;
  ssize_t nread;

  nread = getline(&curLine, &len, mFile);//loop iterates over the file and puts each line into the string curLine
  buf = calloc(strlen(curLine), sizeof(char));
  strcpy(buf, curLine);
  buf[strlen(buf)-1] = '\0';

  fclose(mFile);
  return buf;
}

//Function is same as it is in enc_client
void inputCheck(char* message, char* key) {
  for(int i = 0; i < strlen(message); i++) {
    if((message[i] < 65 || message[i] > 90) && message[i] != 32) {//Checks if out of range of A-Z, ' ' only exception
      fprintf(stderr, "Error, invalid charachters in message\n");
      exit(1);
    }
  }
  
  for(int i = 0; i < strlen(key); i++) {//Does the same on keys
    if((key[i] < 65 || key[i] > 90) && key[i] != 32) {
      fprintf(stderr, "Error, invalid charachters in key\n");
      exit(1);
    }
  }

  if(strlen(key) < strlen(message)) {//Checks to see if key is at least as long as message
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
  port = atoi(argv[3]);

  //Parses the information given in the command line
  message = fileParsed(argv[1]);
  key = fileParsed(argv[2]);
  inputCheck(message, key);
  mesLen = strlen(message)+ strlen(key) + 4;
  messageKey = calloc(mesLen, sizeof(char));
  snprintf(messageKey, mesLen, "%s_%s$$", message, key);


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

  char* connectionCheck = "dec_client";//First notifies that its dec_client
  sendLen = strlen(connectionCheck);
  while((charsWritten = send(socketFD, connectionCheck, sendLen, 0)) < sendLen) {
    if(charsWritten < 0) {
      error("ERROR writing to socket");
    }
    connectionCheck = connectionCheck + charsWritten;
    sendLen = sendLen - charsWritten;
  }
  
  memset(buffer, '\0', sizeof(buffer));
  // Read data from the socket, leaving \0 at end
  charsRead = recv(socketFD, buffer, sizeof(buffer) - 1, 0);
  if (charsRead < 0){
    error("CLIENT: ERROR reading from socket");
  }
  
  if(buffer[0] == 'e') {//If its talking with enc_client then we will write error and quit
    fprintf(stderr, "Cannot connect to this server: dec_server\n");
    exit(2);
  }

  //Sends out the message to decrypt_key with terminating symbol $$ at the end
  sendLen = strlen(messageKey);
  while((charsWritten = send(socketFD, messageKey, sendLen, 0)) < sendLen) {//Loops till all messgae has been sent
    if (charsWritten < 0){
      error("ERROR writing to socket");
    }
    messageKey = messageKey + charsWritten;//Iterates through the word if it didnt fully send
    sendLen = sendLen - charsWritten;
  }

  // Get return message from server
  // Clear out the buffer again for reuse
  memset(completeMessage, '\0', sizeof(completeMessage));
  while(strstr(completeMessage, "@@") == NULL) {//Keep reading until the end of the message has been sent
    memset(buffer, '\0', sizeof(buffer));
    charsWritten = recv(socketFD, buffer, sizeof(buffer)-1, 0);
    strcat(completeMessage, buffer);//Add each segment of the message to the complete message until its done
    if(charsWritten < 0) {//check for errors
      error("ERROR reading from socket\n");
    }
  }
 
  int terminalLocation = strstr(completeMessage, "@@") - completeMessage;//Locate the terminating symbol in the message
  completeMessage[terminalLocation] = '\0';//Replace termiating symbol with null termination
  printf("%s\n", completeMessage);//Print final result to stdout

  // Close the socket
  close(socketFD);
  return 0;
}
