//Really the Keygen
//  client_enc.c
//  Ctesting
//
//  Created by Jackson on 11/22/20.
//  Copyright © 2020 Jackson. All rights reserved.
//

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

//Buffer, mutex, conditional, and needed consumer producer variables for the first communication
int main(int argc, char *argv[]) {
    int length;
    int random;
    srand(time(0)); //seeds random number generators
    
    if (argc < 2) {
        printf("Error insufficient arguments provided\n");
        return EXIT_FAILURE;
    } else {
        length = atoi(argv[1]);
        //printf("recievied length: %d\n", length);
    }
    
    char key[length+1];//Length is known before program is ran bc passed as an argument

    for (int i = 0; i < length; i++) {
        random = rand() % 27;
        if (random == 26) { //Using the last available number as the indicator for a space
            key[i] = ' ';
        } else {
            key[i] = 65 + random;  //65 is added becuase that is the ASCII code for A which is the lowest limit
        }
    }
    key[length] = '\0';
    printf("%s\n", key);
    
    return EXIT_SUCCESS;
}
