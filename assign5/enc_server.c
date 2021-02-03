#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>

//Using a linked list to store the process IDs for concurrency cleanup
//Struct holds the PID and the next node in the list
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

/* This function cleans up the processes that have been running in the background
 * Takes in the linked list of the proccess
 */
void cleanBackground(struct childProccess* list) {
  //struct bacProcList* before = list;
  int returnV = -4; //return stats
  int exitedID = 0; //ID of the child if if done running
  while(list != NULL) {
    exitedID = waitpid(list->processID, &returnV, WNOHANG);//Dont care about the ID just used to cleanup zombies if they exist
    list = list->next;
  }
  return;
}

/* This function encrypts the text of a message with a key
 * This function takes in the message to encrypt and the key to encrypt it with 
 * Returns the encrypted text in the form of a string
 */
char* encryptText(char* message, char* key) {
    char* encryptedMsg = calloc(strlen(message)+2, sizeof(char));//allocates memory for the new string
    int newCharA;
    int newCharB;
    int finalNewChar;
    
    for(int i = 0; i < strlen(message); i++) {
        newCharA = (message[i] - 65);
        newCharB = (key[i] - 65);
        
        if(newCharA < 0) {
            newCharA = 26;
        }
        if(newCharB < 0) {
            newCharB = 26;
        }
        
        finalNewChar = newCharA + newCharB;
        //printf("Final Char: %d\n", finalNewChar);
         
        encryptedMsg[i] = (finalNewChar % 27) + 65;
        if (encryptedMsg[i] == 91) {
            encryptedMsg[i] = ' ';
        }
    }
    strcat(encryptedMsg, "@@");//adds the terminating symbol
    return encryptedMsg;
}

int main(int argc, char *argv[]){
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
    cleanBackground(processList);//First clean up all of the done child processes
    connectionSocket = accept(listenSocket, (struct sockaddr *)&clientAddress, &sizeOfClientInfo);//block until there is a connection wanted
    if (connectionSocket < 0){
      error("ERROR on accept\n");
    }
    childID = fork();//Create a child process for this new connection
    if(childID < 0) {
      error("Error creating new process\n");
    } else if(childID == 0) {//Child process does the actual encryption
      connections = connections + 1;

      memset(messageBuf, '\0', 256);
      charsRead = recv(connectionSocket, messageBuf, 15, 0);//First read in who we are talking to 
      if (charsRead < 0){
        error("ERROR reading from socket\n");
      }

      charsRead = send(connectionSocket, "enc_server", 10, 0);//Next tell them who We are
      if (charsRead < 0){
        error("ERROR writing to socket\n");
      }
      //Only if we are talking with enc_client do we continue
      if(messageBuf[0] == 'e') {//Only need to check the first char becuase that will be what is different between the two servers
        memset(completeMessage, '\0', sizeof(completeMessage));
	//Note that this method for error checking the input was gotten from the explanatino given in class
        while(strstr(completeMessage, "$$") == NULL) {//Keep reading until we find the terminating symbol
          memset(messageBuf, '\0', sizeof(messageBuf));
          charsRead = recv(connectionSocket, messageBuf, sizeof(messageBuf)-1, 0);
          strcat(completeMessage, messageBuf);
          if(charsRead < 0) {
            error("ERROR reading from socket\n");
          }
	}

        int terminalLocation = strstr(completeMessage, "$$") - completeMessage;//Find the terminating symbol location and replace it with null terminating symbol
        completeMessage[terminalLocation] = '\0';
        // Read the client's message from the socket

        plaintext = strtok(completeMessage, "_");//grabs the string token which is the plaintext
        key = strtok(NULL, "_");//Tokenizes again to grab the key
	inputCheck(plaintext, key);
        encryptedText = encryptText(plaintext, key);//Sneds this new information to encrypt the plaintext message
        sendLen = strlen(encryptedText);
        // Send a Success message back to the client
        while((charsRead = send(connectionSocket, encryptedText, sendLen, 0)) < sendLen) {//Send off the encrypted message back to the client, making sure all of it is sent
          if (charsRead < 0){
            error("ERROR writing to socket\n");
          }
          encryptedText = encryptedText + charsRead;
          sendLen = sendLen - charsRead;
        }
        // Close the connection socket for this client
        close(connectionSocket);
      }
      free(encryptedText);
      return EXIT_SUCCESS;//Exits when it is done running
    } else {
      struct childProccess* newProc = malloc(sizeof(struct childProccess));//Parent process adds the new background proccess to the list to check on
      newProc->processID = childID;
      //if(processList == NULL) {//set it as the head or as the head lol realized this at the very end
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
