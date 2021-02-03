#include <dirent.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#define SUFFIX ".csv"
#define PREFIX "movies_"

struct movie { //Only kept the variables I needed for this assignment
  char* name;
  int year;
  //int langCount;
  //char languages[5][20];
  //float rating;
  struct movie* next;
};

void userChoice(int n, const char **arr, int* placeHolder);
char* nameFilter(); //For name of file
char* getFile();
char* getSmallestFile();
char* getLargestFile();
char* getFileNamed(char* fileName); 
struct movie* loadFile(char* fileName);
void getIntLen(int randomNum, int* placeHolder); 
char* createNewDirName(); 
struct movie* newMovie(char* curLine); 
void freeList(struct movie* list); 
void uniqueYears(struct movie* list, char* directory); 
void getSize(struct movie* list, int* placeHolder);
int containsYear(int arr[], int arrSize, int val);
void printList(struct movie* list);

/*The main function runs the main loop of the program, where the file is decided upon 
 *
 *It calls functions to create the dir and the files within it as well as frees all of them after its done with its loop iteration
 *
 */
 
int main(int argc, char *argv[]) {
  int choice = 0; //Users choice
  char* fileToUse = NULL; //name of the file that will later be opened and collected from
  int repeate = 0; //A variable that checks for valid file name for choice 3
  char* newDirName = NULL; //String for the name of the new directory with random number
  char* fileName = NULL; //Stores the file name that the user first enters in for choice 3
  struct movie* movsFromFile = NULL; //Linked list of the movies
  const char* prompArrA[2] = {"1. Select file to process\n", "2. Exit from the program\n"};
  const char* prompArrB[3] = {"\nWhich file you want to process?\nEnter 1 to pick the largest file\n", "Enter 2 to pick the smallest file\n", "Enter 3 to specify the name of a file\n"};


  while(1) {//Repeates forever until the user tells the program they would like to exit
    userChoice(2, prompArrA, &choice);//gets choice from user for continue or quit
    
    if (choice == 1) {
      do {
        userChoice(3, prompArrB, &choice); //gets user choice for which file to use
        if (choice == 1) {
          fileToUse = getLargestFile();
	  repeate = 0; //need to make sure that if user is selecting things again after a failed attempt for choice 3 that the loop wont repeate itself
        } else if (choice == 2) {
	  fileToUse = getSmallestFile();
          repeate = 0;
	} else {
          fileName = nameFilter();//gets user choice for file name
	  fileToUse = getFileNamed(fileName);//checks to see if that file exists or is valid
	  if(fileToUse == NULL) {
	    repeate = 1;
	    printf("file %s does not exist please try again\n\n", fileName);
	  } else {
            repeate = 0;
	  }
          if(fileName != NULL) {
	    free(fileName);//The name of the file from the user was created dynamically so it has to be freed
          }
	}
      } while (repeate);
      
      printf("Processing chosen file named %s\n", fileToUse);
      movsFromFile = loadFile(fileToUse);// shoudl check if its a problem with value sent back being deleted from stack
      //printList(movsFromFile);
      newDirName = createNewDirName(); //creates the new directory to store the file information
      mkdir(newDirName, 0750);//Creates new directory with permissions specified ---------
      printf("Created a new directory named %s\n\n", newDirName);
      uniqueYears(movsFromFile, newDirName);//Function that handles getting the unique years and creating the new files

      //freeing all of the memory that was allocated and used
      if(newDirName != NULL) {
        free(newDirName);
      }
      if(fileToUse != NULL) {
        free(fileToUse);
      }
      if(movsFromFile != NULL) {
        freeList(movsFromFile);
      }
      //movsFromFile = NULL;
      //fileToUse = NULL;
      //newDirName = NULL;
    } else if (choice == 2) {
      return EXIT_SUCCESS;
    }
  }
}

/*Function gets an integer length - realized later that this wasn't necessary because sprintf
 *Takes in the number to check the length of
 */
void getIntLen(int randomNum, int* placeHolder) {
  int i = 0;
  while(randomNum != 0) {
    randomNum = randomNum / 10;
    i++;
  }
  *placeHolder = i;
}

/* Error checking function to see the contents of the linked lists
 *
 */
void printList(struct movie* list) {
  while(list != NULL) {
    printf("%s\n", list->name);
    list = list->next;
  }
}

/*
 *Function responsible for creating the new name of the directory 
 *the first section is already known but the last part needs to be randomly generated 
 *
 */
char* createNewDirName() {
  const char dirName[] = "wrighjac.movies.";//section is constant
  int randN = random() % 99999;//creates random number from 0 - 99999 inclusive
  int randomLen = 0;
  getIntLen(randN, &randomLen);
  int strSize = strlen(dirName) + randomLen;//gets the size of the new string / dir name
  char* newStr = calloc(strSize, sizeof(char)); //creates a placeholder for the user input
  //char newStr[strSize];
  char newNum = ' ';
  //int digit = 0;
  int digitsArr[randomLen];

  for(int i = 1; i <= randomLen; i++) {
    digitsArr[randomLen - i] = randN % 10;//creates an array of the digits that were generated
    randN = randN / 10;
  }

  for(int i = 0; i < strSize; i++) {
    if(i < strlen(dirName)) {
      newStr[i] = dirName[i];
    } else {
      newNum = 48 + digitsArr[i - strlen(dirName)];//48 is the ascii code for 0. By adding the desired number we are getting the ascii code for that given number
      newStr[i] = newNum; //C can convert ints to chars automatically using ascii
    }
  }

  return newStr;
  //printf("file name: %s\n", newStr);

}

//This functino takes in a file name that was given in the command line and parses its data into a linked list of structs
struct movie* loadFile(char* fileName) {
  FILE* movieFile = fopen(fileName, "r");//creates and opens the file with the desire only to read
  int line = 0;

  char* curLine = NULL;//variables that handle the actual reading from file
  size_t len = 0;
  ssize_t nread;
  //char* token;

  struct movie* head = NULL;
  struct movie* tail = NULL;
  struct movie* newMov = NULL;//creates a movie struct given the line in the file

  while((nread = getline(&curLine, &len, movieFile)) != -1) {//loop iterates over the file and puts each line into the string curLine
    //printf(curLine);
    if (line != 0) {
      newMov = newMovie(curLine);//creates a movie struct given the line in the file
    
      if (head == NULL) {//creates the actual linked list of movies
        head = newMov;
	tail = newMov;
      } else {
        tail->next = newMov;
	tail = newMov;
      }
    }
    line++;
  }
  tail->next = NULL;

  free(curLine);
  fclose(movieFile);//closes file
  return head;//returns pointer to the head of the linked list
}

//Function takes in a string that is really a line form a file and parses it into a movie struct
struct movie* newMovie(char* curLine) {
  struct movie* curMovie = malloc(sizeof(struct movie));//allocates memory for the new movie
  
  char* token = strtok_r(curLine, ",", &curLine);//grabs the string token which is ended by the charachter ','
  char* langTok;
  curMovie->name = calloc(strlen(token) + 1, sizeof(char));//only thing that is needed for this program is the name and the year
  strcpy(curMovie->name, token);

  token = strtok_r(NULL, ",", &curLine);
  curMovie->year = atoi(token);

return curMovie;
}

/*Function is responsible for searching the current directory for file with a specified name
 * Takes in a string to search for and return the name if found otherwise it returns NULL to notify to repeate the prompt again
 *
 */
char* getFileNamed(char* fileName) {
  DIR* currDir = opendir(".");
  struct dirent* aDir;
  struct dirent* dirToUse = NULL;
  //int smallestFile = 0;
  struct stat dirStat;
  char* newFileName;

  while((aDir = readdir(currDir)) != NULL) {//Iterates through the dir and checks to see if they are the same length and if they are equal
    if(strlen(aDir->d_name) == strlen(fileName) && (strncmp(aDir->d_name, fileName, strlen(aDir->d_name)) == 0)) {
      dirToUse = aDir;
      break;//if one file matches then wer are done looking
    }
  }
  //printf("file: %s\n\n", dirToUse->d_name);
  //If the file exists then get its name, close the directorty and return the name of the file, otherwise return NULL
  if(dirToUse != NULL) {
    newFileName = calloc(strlen(dirToUse->d_name) + 1, sizeof(char));
    strcpy(newFileName, dirToUse->d_name);
    closedir(currDir);
    return newFileName; 
  }
  closedir(currDir);
  return NULL;
}

/*Function finds the smallest file in the current directory and returns its name
 *Any ties goes to the file that is already held as the smallest file
 */
char* getSmallestFile() {
  DIR* currDir = opendir(".");
  struct dirent* aDir;
  int fthToLast; //Fourth to last is what its supposed to signify (4th char to last) for the file extnesion
  char* name;
  struct dirent* dirToUse = NULL;//allocates memory for the new movie
  int smallestFile = 0;
  struct stat dirStat;

  while((aDir = readdir(currDir)) != NULL) {
    name = aDir->d_name;
    fthToLast = strlen(aDir->d_name) - 4;
    name = name + fthToLast;//Moves the pointer that is pointing to the current file name to point to the 4th to last character
    //Uses this to check if the file is a csv file
    
    if(strncmp(SUFFIX, name, strlen(SUFFIX)) == 0) {//checks the extension
      if(strncmp(PREFIX, aDir->d_name, strlen(PREFIX)) == 0) {//checks the prefix
        stat(aDir->d_name, &dirStat);
	if (dirToUse == NULL || dirStat.st_size < smallestFile) {//If there is no file yet or if the size is smaller than make this the file to use
	  dirToUse = aDir;
	  smallestFile = dirStat.st_size;
	}
      }
    }
  }
  //printf("file: %s\n\n", dirToUse->d_name);
  char* fileName = calloc(strlen(dirToUse->d_name) + 1, sizeof(char));//should be declaring this probably at the top of the file
  strcpy(fileName, dirToUse->d_name);

  closedir(currDir);
  return fileName; 
}

/* Function that checks the current directory for the largest file and returns the name of that file
 * Creates Dynamic memory that is returned for the file they want to use
 */
char* getLargestFile() {
  //printf("first check\n");
  DIR* currDir = opendir(".");
  struct dirent* aDir;
  int fthToLast;
  char* name;
  struct dirent* dirToUse = NULL;//allocates memory for the new movie
  int largestFile = 0;
  struct stat dirStat;
  char* fileName;

  while((aDir = readdir(currDir)) != NULL) {
    name = aDir->d_name;
    fthToLast = strlen(aDir->d_name) - 4;
    name = name + fthToLast;
    
    if(strncmp(SUFFIX, name, strlen(SUFFIX)) == 0) {
      if(strncmp(PREFIX, aDir->d_name, strlen(PREFIX)) == 0) {
	stat(aDir->d_name, &dirStat);//stats function is used to reveal the size of the file in check
	if (dirToUse == NULL || dirStat.st_size > largestFile) {
	  largestFile = dirStat.st_size;
	  dirToUse = aDir;
	}
      }
    }
  }

  //printf("file: %s\n\n", dirToUse->d_name);
  //printf("before copy\n");
  fileName = calloc(strlen(dirToUse->d_name) + 1, sizeof(char));
  strcpy(fileName, dirToUse->d_name);
  //printf("fileName: %s", fileName);
  
  closedir(currDir);
  //printf("first check\n");
  return fileName;//should also try to see if I can just return the name of the file without creating dynamic memory
}

/* Gets input from the user of what file name they want to use
 * Creates dynamic memory to store the user input and 
 */
char* nameFilter() {
  char* langFilter = calloc(50, sizeof(char)); //creates a placeholder for the user input 
  printf("Enter the name of the file for which you wish to proccess: ");
  scanf("%s", langFilter);

  return langFilter;
}

/* Gets the user choice and returns an int
 * This function takes in the number of prompts to print out n as well as an array filled with prompt strings
 * Also checks to see if the number is is within bounds of the choices which is also the number of prompts
 */
void userChoice(int n, const char **arr, int* placeHolder) {
  int valid = 1;
  int choice;

  do {
    if (valid == 0) {
      printf("Invalid input, please try again\n\n");
    }

    for (int i = 0; i < n; i++) {
      printf(arr[i]);
    }
    //printf("1. Select file to process\n");
    //printf("2. Exit from the program\n");
    printf("\nEnter your choice: ");

    scanf("%d", &choice);
    if (choice < 1 || choice > n) {//checks validity of input
      valid = 0;
    } else {
      valid = 1;
    }
  } while(valid == 0);

  *placeHolder = choice;
}

/* This function iterates over the movie list and gets all of the unique years and adds them to an array
 * This array is iterated over and a file is created for every unique year. During this creation the list is iterated over again and each movie
 * that matches the current year is written to the file
 *
 * This function takes in the list to search through as well as the directory to create the files in 
 * Does not return anything but does create new files
 */
void uniqueYears(struct movie* list, char* directory) {
  //int listSize = 100;
  int uniqueYears = 0;
  struct movie* listEnum = list;;
  int file_descriptor;
  int years[50];
  int contains;

  while(listEnum != NULL) {
    contains = containsYear(years, uniqueYears, listEnum->year);
    if (uniqueYears == 0 || !contains) {//If this is a new year then start the search for the highest rating
      years[uniqueYears] = listEnum->year;//adds the year to the list
      uniqueYears++;
    } else {
      listEnum = listEnum->next;
    }
  }

  for(int i = 0; i < uniqueYears; i++) {//Now for each unique year that I have found we want to make a file for that 
    listEnum = list;
    char path[50];//The path is where we want to print the file to wrighjac.movies.nnnnn/year.txt
    snprintf(path, sizeof(path), "%s/%d.txt", directory, years[i]);
    file_descriptor = open(path, O_WRONLY | O_TRUNC | O_CREAT, 0640); //opens a new file descriptor that only for writing with specific permissions given 
    while(listEnum != NULL) {
      if(listEnum->year == years[i]) {//If the years match then print the name of the movies to the file
	char strToAdd[50];
	snprintf(strToAdd, sizeof(strToAdd), "%s\n", listEnum->name);
        write(file_descriptor, &strToAdd, strlen(strToAdd)); 
      }
      listEnum = listEnum->next;
    }
    close(file_descriptor);
  }
  //Send the list of years and the linked list to a function that then prints them out to their own files`
}

/* Gets the size of the movie list and returns it
 * Takes in the list to count from and return its length
 */
void getSize(struct movie* list, int* placeHolder) {
  int count = 0;
  struct movie* listEnum = list;

  while (listEnum != NULL) {
    count++;
    listEnum = listEnum->next;
  }
  *placeHolder = count;
}

/* This fucnction checks to see if an array contains an integer
 * This functino takes in an array to search, the size of the array, and the value to search that array for
 * This returns 1 if the array contains the value and returns 0 if it doesn't'
 */
int containsYear(int arr[], int arrSize, int val) {
  for(int i = 0; i < arrSize; i++) {
    if (arr[i] == val) {
      return 1;
    }
  }

  return 0;
}

/* This function frees a linked list filled with movie structs
 * This function takes in the linked list to free
 * The two things that need to be freed are the name of the movie as well as the movie struct itself.
 */
void freeList(struct movie* list) {
  struct movie* curNode = list;
  struct movie* nextNode;

  if (curNode != NULL && curNode->next != NULL) {
    nextNode = curNode->next;
  }
  
  while(curNode != NULL) {
    free(curNode->name);
    free(curNode);
    curNode = nextNode;
    if (nextNode != NULL) {
      nextNode = nextNode->next;
    }
  }
}
