#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>

#include <dirent.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <math.h>

// This struct is used to store the command parameters that the user enters in 
// This holds the name of the first command, a list of parameters the name of the file to output/ input to if requested
// It also holds an int to tell if it should be ran in the background and lastly the number of parameters
struct userCommand {//Store the input from the user in a struct to make handling the data much easier
  char* name;//This was never actually used as I realized later the name has to be the first in the command line but it was too much work to go through and remove
  char* argl[512];
  char* fileTo;
  char* fileFrom;
  int runBackground;
  int argNum;
};

/* This is my linked list structure that I use to store the ids of the processes that are running in the background
 * List is never deleted from which isn't great if scaled but should be fine for this assignment (saves so much complexity)
 * The list is iterated over, checked if the process is running and moves on from there.
 * List is dynamically allocated and is freed when the user exits the program
 */
struct bacProcList {
  int processID;
  struct bacProcList* next;
};


char* languageFilter(int* commandLen); //Gets the command input from the user and returns it as a string
char* replaceFirst(char* word);
struct userCommand* newCommand(char* curLine);
int builtInFunctions(struct userCommand* uCom, struct bacProcList* list, int exitV); 
void changeDir(struct userCommand* newCom); 
void expandVar(struct userCommand* userComm);
void getSplitC(char* word, int* splitC);
void getIntLen(int num, int* len);
void executeNewProcess(struct userCommand* command);
void cleanBackground(struct bacProcList* list); 
void killBacProc(struct bacProcList* list); 
void freeList(struct bacProcList* list);
void handle_SIGSTP(int signum);


int backgroundMode; //Global Variable that helps keep track on if things should be in background mode


// Sorry for the main being so long. Lots of things depended on the command struct which was established in main 
// I didnt; have time to figure out how to splice things into functions and passing those functions everything they would need to work.
// The main function mostly just handles the forking and the child/ parent process
int main(int argc, char *argv[]) {
  char* newComm = NULL;
  int commandLen = 0;
  int spawnid = -5;
  int childExitStat = 0;
  int forgExit = 0;
  int builtIn = 0;
  struct userCommand* parsedC;
  struct bacProcList* backgroundList = NULL;
  backgroundMode = 1;

  //Set up Signal handler mainly for SIGSTP
  struct sigaction SIGSTP_action = {0}, def_action = {0}, ign_action = {0};
  SIGSTP_action.sa_handler = handle_SIGSTP;
  sigfillset(&SIGSTP_action.sa_mask);
  SIGSTP_action.sa_flags = SA_RESTART;

  //Other actions are mostly default or ignore
  def_action.sa_handler = SIG_DFL;
  ign_action.sa_handler = SIG_IGN;
  sigaction(SIGTSTP, &SIGSTP_action, NULL);
  sigaction(SIGINT, &ign_action, NULL);


  while(1) {
    cleanBackground(backgroundList);//before a command is entered the background process is cleaned
    newComm = languageFilter(&commandLen);

    if(commandLen == 1 || newComm[0] == '#') {//Thought we were supposed to print the line if it was empty or comment but realized not
      //printf("%s\n", newComm);
      //fflush(stdout);
    } else {
      parsedC = newCommand(newComm); //A dynamically allocated command struct has been returned
      expandVar(parsedC);
      builtIn = builtInFunctions(parsedC, backgroundList, forgExit);

	//For all of the other child process of the shell
      if(builtIn == 0) {
	//fflush(stdout);
        spawnid = fork();
	if(spawnid == -1) {
          printf("Error something went wrong\n");
          fflush(stdout);
          exit(1);

	  //Child Process
	} else if(spawnid == 0) {
	  int sourceF;
	  int desF;
	  int result;

	  //Set the propper signal handlers - background functino ignore interupts while foregrounds termiante
	  sigaction(SIGTSTP, &ign_action, NULL);
	  if(parsedC->runBackground) {
	    sigaction(SIGINT, &ign_action, NULL);
	  } else {
	    sigaction(SIGINT, &def_action, NULL);
	  }

	  //Checks for file redirection and adjusts accordingly
	  //Background redirect to null if not specified otherwise
	  //Other than that try and open a file and let the user know if that file is invalid
	  if(parsedC->fileFrom == NULL && parsedC->runBackground) {
	    sourceF = open("/dev/null", O_RDONLY);
	    result = dup2(sourceF, 0);
	  }
	  if(parsedC->fileTo == NULL && parsedC->runBackground) {
	    desF = open("/dev/null", O_WRONLY);
	    result = dup2(desF, 1);
	  }
	  if(parsedC->fileTo != NULL) {
	    desF = open(parsedC->fileTo, O_WRONLY | O_CREAT | O_TRUNC, 0644);
	    if(desF == -1) {
	      perror("Error unable to open file to write to");
	      childExitStat = 1;
	      exit(1);
	    } else {
	      result = dup2(desF, 1);
	    }
	  }
	  if(parsedC->fileFrom != NULL) {
	    sourceF = open(parsedC->fileFrom, O_RDONLY);
	    if(sourceF == -1) {
	      perror("Error unable to open file to write to");
	      childExitStat = 1;
	      exit(1);
	    } else {
	      result = dup2(sourceF, 0);
	    }
	  }

	  //After redirecting is taken care of the command is executed and an arror value is checked for
          execvp(parsedC->argl[0], parsedC->argl);
          perror("Error when executing command:");
          exit(1);
	
	//Parent Process
	} else {
	  //Signal Handlers - Ignore interupts but handle SIGTSTP
	  sigaction(SIGTSTP, &SIGSTP_action, NULL);
	  sigaction(SIGINT, &ign_action, NULL);

	  //Background Process handling
	  //In this we add the id of the background process to a linked list to check on later for zombies/ terminated proc
	  if(parsedC->runBackground && (backgroundMode % 2 == 1)) {
	    struct bacProcList* newProc = malloc(sizeof(struct bacProcList));
            printf("background pid is %d\n", spawnid);
	    fflush(stdout);
	    newProc->processID = spawnid;
	    if(backgroundList == NULL) {//set it as the head or as tail
	      backgroundList = newProc;
	    } else {
	      newProc->next = backgroundList;
	      backgroundList = newProc;
	    }

	  //Foreground Process handling
	  //This part just waits for the process, updates the exit status, and checks for any signals that might have been sent to process
	  } else {
	    waitpid(spawnid, &childExitStat, 0);
	    if(childExitStat != 0) {//sets the appropriate exit value based on how things exited
	      forgExit = 1;
	    } else if(forgExit != 0){
	      forgExit = 0;
	    }
	    if(WIFSIGNALED(childExitStat)) {//checks to see if things were terminated by a signal and sets that signal as status if so
	      forgExit = WTERMSIG(childExitStat);
	      printf("Process was terminated by signal %d\n", forgExit);
	      fflush(stdout);
	    }
	  }
	}
      }
    //Memory was not cleared becuase it kept seg faulting and giving me troublesome errors
    //It really should be in the parent process where this is freed but even that gave me some errors
    //if(parsedC != NULL) {
      //free(parsedC); 
    //}
    //if(newComm != NULL) {
     // free(newComm);
    //}
    }
  }
  return 0;
}

/* This is an error handling functino that takes in a signal that has been sent and prints out a message accordingly, adjusting the necessary values
 * Takes in the signal number that was sent during this call
 */
void handle_SIGSTP(int signum) {
  char* messageA = "SIGSTP signal caught, background mode now disabled\n";
  char* messageB = "SIGSTP signal caught, background mode re-enabled\n";
  if(backgroundMode % 2 == 1) {
    write(STDOUT_FILENO, messageA, 52);
  } else {
    write(STDOUT_FILENO, messageB, 49);
  }
  backgroundMode++;
}

/* This function is reposnsible for executing the built in command
 * This function was added to try and clean up the main
 * This function takes in a command from the user, a list of background processes, and the last exit value
 * This function returns an int which is 1 if a built in function was ran and 0 if not
 */
int builtInFunctions(struct userCommand* uCom, struct bacProcList* list, int exitV) {
  if(strncmp(uCom->name, "exit", strlen(uCom->name)) == 0) {
    //Exxit from the program and kill of process
    killBacProc(list);
    freeList(list);
    exit(0);
  } else if(strncmp(uCom->name, "cd", strlen(uCom->name)) == 0) {
    // Changes Dir and sees if it was successfull
    changeDir(uCom); 
  } else if(strncmp(uCom->name, "status", strlen(uCom->name)) == 0){
    if(exitV > 1) {
      printf("exit signal %d\n", exitV);
      fflush(stdout);
    } else {
      printf("exit value %d\n", exitV);
      fflush(stdout);
    }
    //gets the status of something
    ////For all of the other child process of the shell
  } else {
    return 0;//Let the main know if we need to fork or not basically
  }

  return 1;

}

/* This function is responsible for cleaning up the background processes if they are still running
 * This function gets called from main after looping through a user command
 * This function iterates through a linked list and checks if its done running and cleans it up and prints it out if it is
 */
void cleanBackground(struct bacProcList* list) {
  //struct bacProcList* before = list;
  int returnV = -4; //return stats
  int exitedID = 0; //ID of the child if if done running
  while(list != NULL) {
    exitedID = waitpid(list->processID, &returnV, WNOHANG);
    if(exitedID > 0) {
      if(returnV > 1) {
        printf("background pid %d is done: terminated by signal %d\n", exitedID, returnV);
        fflush(stdout);
      } else {
        printf("background pid %d is done: exit value %d\n", exitedID, returnV);
        fflush(stdout);
      }
    }
    list = list->next;
  }
  return;
}

/* This function is responsible for killing all of the remaining background processes
 * This function is called from main and is passed a linked list of background ids to check 
 *
 */
void killBacProc(struct bacProcList* list) {
  int exitedID = -5;
  int returnV = -5;
  while(list != NULL) {
    exitedID = waitpid(list->processID, &returnV, WNOHANG);
    if(exitedID == 0) { // If the process is still running then kill it 
      kill(list->processID, SIGKILL);
    }
    list = list->next;
  }
  return;
}

/* This function is responsible for clearing the list of background processies
 * It takes in the linked list to clears and it frees all of the memory associated with it
 */
void freeList(struct bacProcList* list) {
  struct bacProcList* buf;
  while(list != NULL) {
    buf = list->next;
    free(list);
    list = buf;
  }
  return;
}

/* This function is responsible for executing a new process (Not currently using it)
 * Takes in a command struct and is supposed to execute the command associated with it
 */
void executeNewProcess(struct userCommand* command) {
  int spawnid;

  spawnid = fork();
  switch(spawnid) {
    case -1:
      printf("Error something went wrong\n");
      fflush(stdout);
      break;

    case 0:
      execvp(command->argl[0], command->argl);
      perror("execv");
      exit(EXIT_FAILURE);
      //exit(1);

    default:
      if(command->runBackground) {
        //bacProcess[bacIn] = spawnid;
	//bacIn++;
	printf("run background\n");
	fflush(stdout);
            
      }
      //waitpid(spawnid, &childExitStat, 0);
      break;
  }

}

//This function is used to get the length of an integer
//Its use in the bigger picture is for storing the process id as a string and for that its length is needed
void getIntLen(int num, int* len) {
  int curLen = 0;
  while(num > 0) {
    num = (int) num / 10;
    curLen++;
  }

  *len = curLen;
  return;
}

//This function takes in a string and replaces the first instance of '$$'.
//This was the easiest way for me to expand the variables
//This function is called repeitidly on the same word until there is no more instances of the variable
char* replaceFirst(char* word) {
  int pid = getpid();
  int pidLen = 0;
  int random = 0;
  getIntLen(pid, &pidLen);
  char pidStr[pidLen+1];
  random = pidLen - 2 + strlen(word); //This is the new length of the word after the id has been expanded
  char* buffer = calloc(random, sizeof(char)); //creates a placeholder for the user input

  snprintf(pidStr, sizeof(pidStr), "%d", pid); //Writes the pid to a string

  for(int i = 0; i < (strlen(word) - 1); i++) {
    if(word[i] == '$' && word[i+1] == '$') {//Checks for the first instance of $$
      if(i == 0) {//If it starts with it then number first and rest of command next should still work if its just $$ since it might just add \0 again
        strcpy(buffer, pidStr);
	word = word + 2;
	strcat(buffer, word);
      } else { //Otherwise its the first part of the command added to the string, then the pid, then the rest if there is any
	strncpy(buffer, word, i);
        strncat(buffer, pidStr, strlen(pidStr));
        if(i + 2 < strlen(word)) {
	  word += i + 2;
	  strcat(buffer, word);
	}
      }
      break;
    }
  }
  return buffer; //returns the new word will have to figure out how to clean up the memory somehow

}

/* This function is responsible for checking if a string contains the var '$$'
 * Returns 1 if there is a variable in the string and 0 if there is not
 * Used by the next function to know when to stop expanding
 */
int containsVar(char* word) {
  for(int i = 0; i < strlen(word) - 1; i++) {
    if(word[i] == '$' && word[i+1] == '$') {
      return 1;
    }
  }
  return 0;
}

/* This function is responsible for expanding the $$ var in each aspect of the command
 * This function takes in a command struct and replaces a substring in it
 * The way it works is by running a loop that replaces the first instance of $$ until there are none left
 * Does this for every parameter in the command if it exists.
 * Doing it this way made it a lot easier to expand the new string while also correctly finding a $$ occurance
 */
void expandVar(struct userCommand* curComm) {
  //char* pidStr[ (int) floor(log10(pid)) + 1]; 
  while(containsVar(curComm->name)) {//first the command name
    curComm->name = replaceFirst(curComm->name);
  }

  if(curComm->argNum > 0) {//next all arguments
    for(int i = 0; i < curComm->argNum; i++) {
      while(containsVar(curComm->argl[i])) {
        curComm->argl[i] = replaceFirst(curComm->argl[i]);
      }
    
    }
  }

  if(curComm->fileTo != NULL) {//next file printing to
    while(containsVar(curComm->fileTo)) {
      curComm->fileTo = replaceFirst(curComm->fileTo);
    }
  }

  if(curComm->fileFrom != NULL) {//Next file from
    while(containsVar(curComm->fileFrom)) {
      curComm->fileFrom = replaceFirst(curComm->fileFrom);
    }
  }
}

/* This function is responsible for getting a command line from the user in the form of a string
 * This function takes in one parameter which is the length of the command that it sets in this function
 * The line is dynamically allocated and returned
 */
char* languageFilter(int* commandLen) {
  size_t bufSize = 2048;
  char* buffer = calloc(2048, sizeof(char)); //creates a placeholder for the user input
  size_t realSize;
  printf(": ");
  fflush(stdout);

  realSize = getline(&buffer, &bufSize, stdin);
  buffer[strlen(buffer)-1] = 0;
  (*commandLen) = strlen(buffer);

  return buffer;
}

/* This function is responsible for parsing a command line from the user into a command struct
 * This function takes in a string to parse and returns a pointer to the new struct
 *
 */
struct userCommand* newCommand(char* curLine) {
  struct userCommand* curComm = malloc(sizeof(struct userCommand));//allocates memory for the new command
  char* cd_name = "cd";
  char* token = strtok_r(curLine, " ", &curLine);//grabs the string token which is ended by the charachter ','
  char* langTok;
  int argLen = 0;
  curComm->name = calloc(strlen(token) + 1, sizeof(char));
  strcpy(curComm->name, token);

  curComm->argl[argLen] = calloc(strlen(token) + 1, sizeof(char));//first element of the argument list is the command itself
  strcpy(curComm->argl[argLen], token);
  argLen++;

  curComm->runBackground = 0;

  while((token = strtok_r(NULL, " ", &curLine)) != NULL) {//command is split up by spaces
    if(strncmp(token, "<", strlen(token)) == 0) { //if there is a file from then deal with it specially
      token = strtok_r(NULL, " ", &curLine);
      curComm->fileFrom = calloc(strlen(token) + 1, sizeof(char));
      strcpy(curComm->fileFrom, token);
    } else if(strncmp(token, ">", strlen(token)) == 0) {//if there is a file to then deal with it specially
      token = strtok_r(NULL, " ", &curLine);
      curComm->fileTo = calloc(strlen(token) + 1, sizeof(char));
      strcpy(curComm->fileTo, token);
    } else if(strncmp(token, "&", strlen(token)) == 0) {//note if its supposed to be ran in the background
      if(strtok_r(NULL, " ", &curLine) == NULL) {
        curComm->runBackground = 1;
      }
    } else { //otherwise add it to the argument list
      curComm->argl[argLen] = calloc(strlen(token) + 1, sizeof(char));
      strncpy(curComm->argl[argLen], token, strlen(token));//wa
      argLen++;
    }
    curComm->argl[argLen] = NULL;//last argument needs to be null in the command struct
  }

  curComm->argNum = argLen;

  return curComm;
}

/* This functino is reponsible for changing the current directory
 * This fucntion takes in a command struct from the user to execute
 * Returns nothing but changes directory to either the home if one isnt specified or to the path they want
 *
 */
void changeDir(struct userCommand* newCom) {
  int builtInRes = 0;
  //char mainPath[50];
  
  if(newCom->argNum == 1) {
    builtInRes = chdir(getenv("HOME"));
  } else {
    builtInRes = chdir(newCom->argl[1]);
  }

  if(builtInRes == -1) {
    printf("Invalid pathname given failed to change directories\n");
    fflush(stdout);
  }
  return;
}
