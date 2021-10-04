# Operating-Systems-1
Codes that was developed for my operating systems class <br/>
Index of assignments:<br/>

Assignment 1 is a program that reads in a file for movies, stores them in a struct, then organizes them based on user preferences. (Assignment focused on files)<br/>

Assignment 2 is a program that reads in a file that the user can choose in the current directory, then it crates a file for ever year in the input file and puts it in a new directory created by the program (Assignment focused on files and directories)<br/>

Assignment 3 is a program that is a basic recreation of a shell. This allows the user to run commands that are found on the normal shell such as "ls" "cd" "ping" etc. The program also allows the user to redirect output, take input, and run a program in the background. Each command is run on its own process created by the program, which is monitored by the program and propperly cleaned up. Background programs are also minotored and cleaned when finished to prevent zombie-processes<br/>

Assignment 4 is a multi-threaded program where one thread reads in input from std-in, one thread replaces each '\n' with a space, one thread replaces every '++' with a '^', and the last thread outputs to std-out with 80 charachters per line. The program uses mutual exlusion to avoid race conditions found with sharing resources.<br/>

Assignment 5 is a server and client program where programs talk to each other through ports on the local machine. This program takes in a file through the encoding client, which then contact the encoding server and passes it a file to encode. This server then creates a newly encoded file. The decoded server then gives a port and file which then contacts the decoding server which decodes the given file and outputs the result to a new file<br/>

Assignment 6 is a multi-threaded program in rust which reads in a number of partitions from the user, creates a thread for each partition that generates random numbers and collects the sum for each partition adding all the sums at the end for the total sum.<br/>
