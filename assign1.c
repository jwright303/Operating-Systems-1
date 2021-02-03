#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Movie struct which includes its own info along with the next node in the link
struct movie {
  char* name;
  int year;
  int langCount;
  char languages[5][20];
  float rating;
  struct movie* next;
};

//List of all the headers for the functions used in this program
int userChoice();
void moviesByYear(struct movie* list);
int moviesByRating();
void moviesByLang(struct movie* list);
struct movie* loadFile();
struct movie* newMovie(char* curLine);
void printAll(struct movie* list);
char* languageFilter();
int getSize(struct movie* list);
int containsYear(int arr[], int arrSize, int val); 
struct movie* topForYear(struct movie* startNode); 
void freeList(struct movie* list);

int main(int argc, char *argv[]) {
  if (argc < 2) {//If the user doesn't enter at least a file to parse then the program will quit automatically
    printf("File to be parsed was not given, program terminating...\n");
    return EXIT_FAILURE;
  } else {
    int choice = 0;

    struct movie* list = loadFile(argv[1]); //Loads the file given and creates a pointer to the head of the list
    //printAll(list);

    while(1) {//Repeates forever until the user tells the program they would like to exit
      choice = userChoice();//gets choice from user
      if (choice == 1) {
        moviesByYear(list); 
      } else if (choice == 2) {
        moviesByRating(list);
      } else if (choice == 3) {
        moviesByLang(list);
      } else {
	freeList(list);
        return EXIT_SUCCESS;
      }
    }
  }

}

//This functino takes in a file name that was given in the command line and parses its data into a linked list of structs
struct movie* loadFile(char* fileName) {
  FILE* movieFile = fopen(fileName, "r");//creates and opens the file with the desire only to read
  int line = 0;

  char* curLine = NULL;
  size_t len = 0;
  ssize_t nread;
  char* token;

  struct movie* head = NULL;
  struct movie* tail = NULL;

  while((nread = getline(&curLine, &len, movieFile)) != -1) {//loop iterates over the file and puts each line into the string curLine
    //printf(curLine);
    if (line != 0) {
      struct movie* newMov = newMovie(curLine);//creates a movie struct given the line in the file
    
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
  printf("\nProcessed file %s and parsed data for %d movies\n\n", fileName, line-1);
  free(curLine);
  fclose(movieFile);//closes file
  return head;//returns pointer to the head of the linked list
}

//Creates a new movie struct given a string which is a line from the file given earlier
struct movie* newMovie(char* curLine) {
  struct movie* curMovie = malloc(sizeof(struct movie));//allocates memory for the new movie
  
  char* token = strtok_r(curLine, ",", &curLine);//grabs the string token which is ended by the charachter ','
  char* langTok;
  curMovie->name = calloc(strlen(token) + 1, sizeof(char));
  strcpy(curMovie->name, token);

  token = strtok_r(NULL, ",", &curLine);
  curMovie->year = atoi(token);

  token = strtok_r(NULL, ",", &curLine);
  langTok = strtok_r(token, "[;]", &token);//This token itself is split into segments for each language there is adding them to an array 
  strcpy(curMovie->languages[0], langTok);
  int c = 1;
  while((langTok = strtok_r(NULL, "[;]", &token))) {
    strcpy(curMovie->languages[c], langTok);
    c++;
  }
  curMovie->langCount = c;

  token = strtok_r(NULL, ",", &curLine);//gets the last token from the line which is the rating of the moive
  char* ptr;
  curMovie->rating = strtod(token, &ptr);

return curMovie;
}

//This function gets the choice of how to filter the movies from the user. This only accepts integers from 1-4
//Ints out of range will receive a error message
int userChoice() {
  int valid = 1;
  int choice;

  do {
    if (valid == 0) {
      printf("Invalid input, please try again\n\n");
    }

    printf("1. Show movies released in specified year\n");
    printf("2. Show highest rated movie for each year\n");
    printf("3. Show the title and year of release of all movies in specific language\n");
    printf("4. Exit from the program\n");
    printf("\nEnter your choice: ");

    scanf("%d", &choice);
    if (choice < 1 || choice > 4) {//checks validity of input
      valid = 0;
    } else {
      valid = 1;
    }
  } while(valid == 0);
  return choice;
}

//Function was just used in tests to print out all of the information about each movie
void printAll(struct movie* list) {
  while(list != NULL) {
    printf("%s, %d, %.1f ", list->name, list->year, list->rating);
    for(int i = 0; i < list->langCount; i++) {
      printf("%s ", list->languages[i]);
    }
    printf("%d\n", list->langCount);
    list = list->next;
  }
}

//This gets movies given a specific year
void moviesByYear(struct movie* list) {
  int year;
  int movsInYear = 0;
  printf("Enter the year of which you would like to see movies for: ");

  scanf("%d", &year);
  while(list != NULL) {//Iterates through the linked list and if the movie dates match then the movie info is printed out
    if (list->year == year) {
      printf("%s\n", list->name);
      movsInYear++;
    }
    list = list->next;
  }

  if (movsInYear == 0) {//If there are no movies with the given date then a special message is printed
    printf("There were no movies from the list during this year");
  }
  printf("\n");
  
  return;
}

//This function gets the size of the linked list. This is used to keep track of the unique years which must be less than or equal to the number of movies
//This function takes in a linked list and returns the count of it
int getSize(struct movie* list) {
  int count = 0;
  while (list != NULL) {
    count++;
    list = list->next;
  }
  return count;
}

//This function checks to see if an array contains a given value. This is used to check to see if the rating has already been printed for that given year
//This function takes in an array to check, the size of the array, and the value to search for
//Returns 1 for true if the array contains the value and 0 for false
int containsYear(int arr[], int arrSize, int val) {
  int contains = 0;
	
  for(int i = 0; i < arrSize; i++) {
    if (arr[i] == val) {
      contains = 1;
      break;
    }
  }

  return contains;
}

//Once a unique year is found this function is called to search for the highest rating for that year
//This function takes in a node in a linked list to start with and returns the movie with the highest rating
struct movie* topForYear(struct movie* startNode) {
  struct movie* topRated = startNode; //Pointer node which points to the highest rated movie
  int curYear = startNode->year; //Only compares the node with movies that are in the same year

  if (startNode->next == NULL) {
    return startNode;//If there is a node at the end of the list with a unique year then that will be the highest rated movie
  } else {
    struct movie* movieEnum = startNode->next;
    while(movieEnum != NULL) {
      if ((movieEnum->year == curYear) && (movieEnum->rating > topRated->rating)) {//switches the top rated pointer if there is a higher rated movie in sme year
        topRated = movieEnum;
      }
      movieEnum = movieEnum->next;
    }
    return topRated;
  }
}

//This function prints out movies with the highest rating in each year
//It takes in a linked list and prints out the movies
int moviesByRating(struct movie* list) {
  int listSize = getSize(list);
  int years[listSize];
  int uniqueYears = 0;
  struct movie* topRated;  

  while(list != NULL) {
    if (uniqueYears == 0 || !containsYear(years, uniqueYears, list->year)) {//If this is a new year then start the search for the highest rating
      years[uniqueYears] = list->year;//adds the year to the list
      uniqueYears++;
      topRated = topForYear(list);
      printf("%d %.2f %s\n", topRated->year, topRated->rating, topRated->name);
    } else {
      list = list->next;
    }
  }
  printf("\n");
  
  return 0;
}

//This function gets a string from the user which is used to filter movies by language
//Takes no parameters but returns a string (array of chars)
char* languageFilter() {
  char* langFilter = calloc(20, sizeof(char)); //creates a placeholder for the user input
  printf("Enter the language for which you would like to see movies for: ");
  scanf("%s", langFilter);

  return langFilter;
  free(langFilter);
}

//This function prints out the movies by language given a filter from the user
//This function takes a linked list to filter from
void moviesByLang(struct movie* list) {
  char* langFilter = languageFilter(); //Gets the language to filter from the user
  int movCount = 0; //checks the number of movies for the language given

  while(list != NULL) {//Iterates through the list and prints out the values if it matches the language filter
    for (int i = 0; i < list->langCount; i++) {
      if (strcmp(langFilter, list->languages[i]) == 0) {
        printf("%d %s\n", list->year, list->name);
	movCount++;
	break;
      }
    }
    list = list->next;
  }
  if(movCount == 0) {//If there are no movies that match the language then let the user know.
    printf("No data about movies made in %s", langFilter);
  }
  printf("\n");

  free(langFilter); //after if filters the movies by the langue it frees the language variable that was used to get input from user

  return;
}

//This function frees the linked list that was created for this program
//IT takes in the linked list to be freed and frees it based off of the structure of the struct of the linked list.
void freeList(struct movie* list) {
  struct movie* curNode = list;
  struct movie* nextNode = list->next;
  
  while(curNode != NULL) {
    free(curNode->name);
    free(curNode);
    curNode = nextNode;
    if (nextNode != NULL) {
      nextNode = nextNode ->next;
    }
  }
}

