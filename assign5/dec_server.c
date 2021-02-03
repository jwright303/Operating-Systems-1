#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>

//Same as enc_server, this uses a linked list to store PID of child proccess.
struct childProccess {
  int processID;
  struct childProccess* next;
};

// Error function used for reporting issues
void error(const char *msg) {
  perror(msg);
  exit(1);
}

// Set up the address struct for the server socket
void setupAddressStruct(struct sockaddr_in* address, int portNumber){
 
  // Clear out the address struct
  memset((char*) address, '\0', sizeof(*address));

  // The address should be network capable
  address->sin_family = AF_INET;
  // Store the port number
  address->sin_port = htons(portNumber);
  // Allow a client at any address to connect to this server
  address->sin_addr.s_addr = INADDR_ANY;
}

void cleanBackground(struct childProccess* list) {
  //struct bacProcList* before = list;
  int returnV = -4; //return stats
  int exitedID = 0; //ID of the child if if done running
  while(list != NULL) {
    exitedID = waitpid(list->processID, &returnV, WNOHANG);
    list = list->next;
  }
  return;
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
    }
  }
  
  for(int i = 0; i < strlen(key); i++) {
    if((key[i] < 65 || key[i] > 90) && key[i] != 32) {//Checks same for the key
      fprintf(stderr, "Error, invalid charachters in key\n");
    }
  }

  if(strlen(key) < strlen(message)) {//Checks if the key is shorter than the message
    fprintf(stderr, "Error, key is too short\n");
  }
}

/* This function decrypts the message with a key
 * This function takes in a string to decrypt and a string to use as the key 
 * This function returns a decrypted string
 */
char* decryptText(char* message, char* key) {
  char newCharA;
  char newCharB;
  char finalChar;
  char* decryptedMsg = calloc(strlen(message)+2, sizeof(char));//Create a new string to store this message in

  for(int i = 0; i < strlen(message); i++) {
    newCharA = message[i] - 65;//Now A = 0, Z = 25, ' ' = 26
    newCharB = key[i] - 65;

    if(newCharA < 0) {
      newCharA = 26;
    }

    if(newCharB < 0) {
      newCharB = 26;
    }

    finalChar = newCharA - newCharB;//Do the decryption as outlined in the assignment
    if(finalChar < 0) {
      finalChar = finalChar + 27;
    }
    if(finalChar == 26) {
      decryptedMsg[i] = ' ';
    } else {
      decryptedMsg[i] = (finalChar%27) + 65;
    }
  }
  strcat(decryptedMsg, "@@");//Add null terminating symbol at the end of the message

  return decryptedMsg;
}

int main(int argc, char *argv[]){//In all functionality except for encryption this is the same as enc_server, so it has been coppied over from there
  int connectionSocket, charsRead, sendLen, connections = 0, childID;
  char messageBuf[1000], completeMessage[200000];
  char* plaintext;
  char* key;
  char* encryptedText;
  struct sockaddr_in serverAddress, clientAddress;
  struct childProccess* processList = NULL;
  socklen_t sizeOfClientInfo = sizeof(clientAddress);

  // Check usage & args
  if (argc < 2) {
    printf("arg count: %d\n", argc);
    fprintf(stderr,"USAGE: %s port\n", argv[0]);
    exit(1);
  }
  
  int listenSocket = socket(AF_INET, SOCK_STREAM, 0); //// Create the socket that will listen for connections
  if (listenSocket < 0) {
    error("ERROR opening socket\n");
  }

  setupAddressStruct(&serverAddress, atoi(argv[1]));// Set up the address struct for the server socket

  if (bind(listenSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0){// Associate the socket to the port
    error("ERROR on binding\n");
  }

  listen(listenSocket, 5);// Start listening for connetions. Allow up to 5 connections to queue up
  
  while(1){// Accept a connection, blocking if one is not available until one connects
    // Accept the connection request which creates a connection socket
    cleanBackground(processList);
    connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);//Accept is a blocking call which makes it so there is not a fork bomb
    if (connectionSocket < 0){
      error("ERROR on accept\n");
    }
    childID = fork();//Each new connection gets a child process
    if(childID < 0) {//Error check
      error("Error creating new process\n");
    } else if(childID == 0) {//Child process section
      connections = connections + 1;

      //First receives who its talking to 
      memset(messageBuf, '\0', 256);
      charsRead = recv(connectionSocket, messageBuf, 15, 0);
      if (charsRead < 0){
        error("ERROR reading from socket\n");
      }
      
      //Then tells them who this server is
      charsRead = send(connectionSocket, "dec_server", 10, 0);
      if (charsRead < 0){
        error("ERROR writing to socket\n");
      }
      //Only runs the rest if its talking with a dec_client
      if(messageBuf[0] == 'd') {//Only need to check the first char becuase that will be what is different between the two servers
        memset(completeMessage, '\0', sizeof(completeMessage));
        while(strstr(completeMessage, "$$") == NULL) {//Now receives the message from the client until the terminating symbil is received
          memset(messageBuf, '\0', sizeof(messageBuf));
          charsRead = recv(connectionSocket, messageBuf, sizeof(messageBuf)-1, 0);
          strcat(completeMessage, messageBuf);
          if(charsRead < 0) {
            error("ERROR reading from socket\n");
          }
        }

        int terminalLocation = strstr(completeMessage, "$$") - completeMessage;
        completeMessage[terminalLocation] = '\0';
        // Read the client's message from the socket

        plaintext = strtok(completeMessage, "_");//Tokenizes out the message and the key
        key = strtok(NULL, "_");
	inputCheck(plaintext, key);
        encryptedText = decryptText(plaintext, key);//Uses the message it has been passed to decrypt the message
        sendLen = strlen(encryptedText);
        // Send a Success message back to the client
        while((charsRead = send(connectionSocket, encryptedText, sendLen, 0)) < sendLen) {//Returns the decrypted message and makes sure all of it is sent
          if (charsRead < 0){
            error("ERROR writing to socket\n");
          }
          encryptedText = encryptedText + charsRead;
          sendLen = sendLen - charsRead;
        }
        //charsRead = send(connectionSocket, encryptedText, strlen(encryptedText), 0);
        // Close the connection socket for this client
        close(connectionSocket);
        }
        return EXIT_SUCCESS;//Kill off the child process when its done with a success message
      
      //Mother Process
      } else {
        struct childProccess* newProc = malloc(sizeof(struct childProccess));//Add the background child to a ID list to check on later
        newProc->processID = childID;
        //if(processList == NULL) {//set it as the head or as tail
          //processList = newProc;
        //} else {
          newProc->next = processList;
          processList = newProc;
        //}
      }
  }
  // Close the listening socket
  close(listenSocket);
  return 0;
}
