#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <pthread.h>
#include <math.h>
#include <unistd.h>
#include <unistd.h>

//Buffer, mutex, conditional, and needed consumer producer variables for the first communication
//Note that the buffer is an array of pointers to strings
//This made it significantly easier to implament the consumer producer approach becuase I could just pass around the pointers instead of having to pass around groups of chars
char* buff_1[50];
int count_1 = 0;
int proIn_1 = 0;
int conIn_1 = 0;
pthread_mutex_t mutex_1 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full_1 = PTHREAD_COND_INITIALIZER;

//Buffer, mutex, conditional, and needed consumer producer variables for the second communication
char* buff_2[50];
int count_2 = 0;
int proIn_2 = 0;
int conIn_2 = 0;
pthread_mutex_t mutex_2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full_2 = PTHREAD_COND_INITIALIZER;

//Buffer, mutex, conditional, and needed consumer producer variables for the third communication
char* buff_3[50];
int count_3 = 0;
int proIn_3 = 0;
int conIn_3 = 0;
int chars_3 = 0;
pthread_mutex_t mutex_3 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t full_3 = PTHREAD_COND_INITIALIZER;

//Special Buffer and needed variables for the final printing Since there can at most be 50 lines of 1000 chars the most 80 len lines you can make is 612.5
char print_buf[613][81];
int char_in = 0;
int comp_lines = 0;
int lineIn = 0;
int lineC = 0;
pthread_mutex_t mutex_4 = PTHREAD_MUTEX_INITIALIZER;

//functions that the threads use to complete thier tasks
char* newLine(void);
void replaceLineSep(char* line);
char* replacePlusSigns(char* line);
void putBuff_1(char* input);
void parseLineToBuff(char* line);

//Functions where the threads will be running
void *writeOutput(void* args);
void *replaceLineS(void* args);
void *replacePlusS(void* args);
void *getInput(void* args);

//Functions that handle putting and retreiving information from the buffers
//Note: These functions were the ones given to us by Justin in the Assignment page on canvas and were only slightly modified
void putBuff_1(char* input);
char* getBuff_1(void);
void putBuff_2(char* input);
char* getBuff_2(void);
void putBuff_3(char* input);
char* getBuff_3(void);


int main(int argc, char *argv[]) {
    srand(time(0));
    pthread_t input_t, plus_sign, line_sep, output_t;
    
    // Create the threads
    pthread_create(&input_t, NULL, getInput, NULL);
    pthread_create(&plus_sign, NULL, replaceLineS, NULL);
    pthread_create(&line_sep, NULL, replacePlusS, NULL);
    pthread_create(&output_t, NULL, writeOutput, NULL);

    // Wait for the threads to terminate
    pthread_join(input_t, NULL);
    pthread_join(plus_sign, NULL);
    pthread_join(line_sep, NULL);
    pthread_join(output_t, NULL);
    
    return EXIT_SUCCESS;
}

/*
 Function is run by a thread which gets input for up to 50 lines that are up to 1000 chars in length
 Returns Null since just used by a thread
 */
void *getInput(void* args) {
    char* input;
    for (int i = 0; i < 50; i++) {
        input = newLine();
        //instead have newLine return something special when STOP is detected that way we can forward it on and break each of the loops
        putBuff_1(input);
        if (input == NULL) {//Only happens when the user entered STOP
            return NULL;
        }
    }
    return NULL;
}

/*
 Function puts item into buffer 1. Function is a slightly modified version of what was given to us by Justin in the Assignment page
 Essentially locks the mutex, puts an item in the buffer, adjusts the corresponding variables, signlas the buffer isnt empty, and unlocks mutex
 This function is the same for each of the different buffers
 */
void putBuff_1(char* input) {
    pthread_mutex_lock(&mutex_1);
    // Put the item in the buffer
    buff_1[proIn_1] = input;
    // Increment the index where the next item will be put.
    proIn_1 = proIn_1 + 1;
    count_1++;
    // Signal to the consumer that the buffer is no longer empty
    pthread_cond_signal(&full_1);
    // Unlock the mutex
    pthread_mutex_unlock(&mutex_1);
}

/*
 Function gets item from buffer 1. Function is a slightly modified version of what was given to us by Justin in the Assignment page
 Essentially locks the mutex, checks/waits for an empty buffer, gets an item from the buffer, adjust the according variables, and unlocks mutex
 This function is the same for each of the different buffers
 */
char* getBuff_1() {
    pthread_mutex_lock(&mutex_1);
    while (count_1 == 0) {
      // Buffer is empty. Wait for the producer to signal that the buffer has data
        pthread_cond_wait(&full_1, &mutex_1);
    }
    char* item = buff_1[conIn_1];
    // Increment the index from which the item will be picked up
    conIn_1 = conIn_1 + 1;
    count_1--;
    // Unlock the mutex
    pthread_mutex_unlock(&mutex_1);
    // Return the item
    return item;
}

void putBuff_2(char* input) {
    pthread_mutex_lock(&mutex_2);
    // Put the item in the buffer
    buff_2[proIn_2] = input;
    // Increment the index where the next item will be put.
    proIn_2 = proIn_2 + 1;
    count_2++;
    // Signal to the consumer that the buffer is no longer empty
    pthread_cond_signal(&full_2);
    // Unlock the mutex
    pthread_mutex_unlock(&mutex_2);
}

char* getBuff_2() {
    pthread_mutex_lock(&mutex_2);
    while (count_2 == 0) {
      // Buffer is empty. Wait for the producer to signal that the buffer has data
        pthread_cond_wait(&full_2, &mutex_2);
    }
    char* item = buff_2[conIn_2];
    // Increment the index from which the item will be picked up
    conIn_2 = conIn_2 + 1;
    count_2--;
    // Unlock the mutex
    pthread_mutex_unlock(&mutex_2);
    // Return the item
    return item;
}

void putBuff_3(char* input) {
    pthread_mutex_lock(&mutex_3);
    // Put the item in the buffer
    buff_3[proIn_3] = input;
    if (input != NULL) {
        chars_3 = chars_3 + (int)strlen(input);
    }
    // Increment the index where the next item will be put.
    proIn_3 = proIn_3 + 1;
    count_3++;
    // Signal to the consumer that the buffer is no longer empty
    pthread_cond_signal(&full_3);
    // Unlock the mutex
    pthread_mutex_unlock(&mutex_3);
}

char* getBuff_3() {
    pthread_mutex_lock(&mutex_3);
    while (count_3 == 0) {
      // Buffer is empty. Wait for the producer to signal that the buffer has data
        pthread_cond_wait(&full_3, &mutex_3);
    }
    char* item = buff_3[conIn_3];
    // Increment the index from which the item will be picked up
    conIn_3 = conIn_3 + 1;
    count_3--;
    // Unlock the mutex
    pthread_mutex_unlock(&mutex_3);
    // Return the item
    return item;
}

/*
 Function is ran by a thread, takes no real arguments.
 Retreives line from the buffer and replaces the plus signs if its not NULL (only NULL when STOP is entered)
 */
void *replacePlusS(void* args) {
    char* newLine;
    for (int i = 0; i < 50; i++) {
        newLine = getBuff_2();
        if(newLine != NULL) {
            newLine = replacePlusSigns(newLine);
        }
        putBuff_3(newLine);
        if (newLine == NULL) {//Thread terminates when STOP is entered
            return NULL;
        }
    }
    return NULL;
}

/*
Function is ran by a thread, takes no real arguments.
Retreives line from the buffer and replaces the new line symbols when its not NULL (only NULL when STOP is entered)
*/
void *replaceLineS (void* args) {
    char* item;
    for (int i = 0; i < 50; i++) {
        item = getBuff_1();
        if(item != NULL) {
            replaceLineSep(item);
        }
        putBuff_2(item);
        if (item == NULL) {//Thread will end when STOP is entered
            return NULL;
        }
    }
    return NULL;
}

/*
Function is ran by a thread, takes no real arguments.
Retreives line from the buffer and adds it to the new parsed buffer which will be easier to read from
*/
void *writeOutput(void* args) {
    char* buff;
    for (int i = 0; i < 50; i++) {
        buff = getBuff_3();
        if (buff != NULL) {
            parseLineToBuff(buff);
        }
        
	//pthread_mutex_lock(&mutex_4);
        while (lineC > 0) { //lineC is the count of completed lines of the new buffer which each have 80 chars and 1 newline char so only prints when there is a completed line
            write(STDOUT_FILENO, print_buf[lineIn], sizeof(char) * 81); // write has to be used beucase print is expecting a \0 at the end which runs into erros
            lineIn = lineIn + 1; //Iterates to the next line
            lineC = lineC - 1; //Reduces the count of 80 char lines
        }
	//pthread_mutex_unlock(&mutex_4);

        if (buff == NULL) { //Termintes after printing all of the 80 char lines possible
            return NULL;
        }
    }
    return NULL;//Dont really think this will ever execute unless maybe more than 50 lines are entered
}

/*
 Function takes in a string and parses it into a buffer that each has 81 chars and many lines
 (49 * 1000 / 80 = 612.5)  --  print_buf[613][81]
 Note that the 50th line will be the stop line which isnt printed to the screen
 */
void parseLineToBuff(char* line) {
    int lineLen = (int)strlen(line);
    
    for (int i = 0; i < lineLen; i++) {
        print_buf[comp_lines][char_in] = line[i]; //Goes char by char which is the easiest but not the most efficient way
        char_in = char_in + 1;
        if (char_in == 80) {
            print_buf[comp_lines][char_in] = '\n';
            comp_lines = comp_lines + 1;
            char_in = 0;
	    //pthread_mutex_lock(&mutex_4);//Dont think this is really needed becuase only one thread al
            lineC = lineC + 1;
	    //pthread_mutex_unlock(&mutex_4);
	    //printf("lines: %d\n", lineC);
        }
    }
}

//Function used by the input thread to get a line of up to 1000 chars form the user
char* newLine (void) {
    size_t bufSize = 1000;
    char* buffer = calloc(bufSize, sizeof(char)); //creates a placeholder for the user input
    size_t realSize;
  
    realSize = getline(&buffer, &bufSize, stdin);
    
    if(strncmp(buffer, "STOP", 4) == 0 && strlen(buffer) == 5) {
        return NULL; //NULL will never be entered as input so it can act as a signifier to end the program and print things
    }
    return buffer;
}

/*
 Function replaces the instances newline charachters with spaces
 The function takes in a string to modify and returns the altered string
 */
void replaceLineSep(char* line) {
    unsigned long len = strlen(line);
    
    //Very simple char replacement
    for (int i = 0; i < len; i++) {
        if(line[i] == '\n') {
            line[i] = ' ';
        }
    }
    return;
}

/*
 Function takes in a line and replaces all of the plus signs
 First it checks to see how many there are to replace then creates a new string of appropriate new size and then fills it with the new content
 */
char* replacePlusSigns(char* line) {
    unsigned long len = strlen(line);
    int plusCount = 0; //Used to allocate memory for the new string
    int newLineIn = 0; //Used to keep track of where we are in the new altered string
    char* expandedLine; //Pointer to what will be a new string with just enough memory for the converted string
  
  //If really wanted to we could just make a new string of the same length of the one given and skip this for loop
    for (int i = 0; i < len; i++) {
        if((i + 1) <= len) {
            if(line[i] == '+' && line[i+1] == '+') {
                plusCount++;
                i++;//dont want to check the next char since we know it will already be +
            }
            
        } else {
            break;
        }
    }
    if (plusCount == 0) {
        return line;
    }
      
    expandedLine = calloc((len - plusCount), sizeof(char));
      
    for (int i = 0; i < len; i++) {
        if((i + 1) <= len) {
            if(line[i] == '+' && line[i+1] == '+') {
                expandedLine[newLineIn] = '^'; //replaces the instance of ++ with ^
                newLineIn++;
                i++;//dont want to check the next char since we know it will already be
            } else {
                expandedLine[newLineIn] = line[i];
                newLineIn++;
            }
        } else {
            break;
        }
    }
    return expandedLine;

}
