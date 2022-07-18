# Operating-Systems-1
Codes that was developed for my operating systems class <br/>
Index of assignments:<br/>

### Assignment 1 sorting
Assignment 1 is a program that reads in a file for movies, stores them in a struct, then organizes them based on user preferences. (Assignment focused on files)<br/>

### Assignment 2 directories
Assignment 2 is a program that reads in a file that the user can choose in the current directory, then it crates a file for ever year in the input file and puts it in a new directory created by the program (Assignment focused on files and directories)<br/>

### Assignment 3 mini shell
Assignment 3 is a program that is a basic recreation of a shell. This allows the user to run commands that are found on the normal shell such as "ls" "cd" "ping" etc. The program also allows the user to redirect output, take input, and run a program in the background. Each command is run on its own process created by the program, which is monitored by the program and propperly cleaned up. Background programs are also minotored and cleaned when finished to prevent zombie-processes<br/>
<img width="691" alt="Screen Shot 2022-07-18 at 11 56 24 AM" src="https://user-images.githubusercontent.com/41707123/179583283-50ca2770-59a7-428a-8bee-932aeaeb4ce2.png">


### Assignment 4 multi-threading
Assignment 4 is a multi-threaded program where one thread reads in input from std-in, one thread replaces each '\n' with a space, one thread replaces every '++' with a '^', and the last thread outputs to std-out with 80 charachters per line. The program uses mutual exlusion to avoid race conditions found with sharing resources. To compile run: `gcc -pthread -std=gnu99 assign4.c -o a4`<br/>
<img width="1421" alt="Screen Shot 2022-07-18 at 12 06 22 PM" src="https://user-images.githubusercontent.com/41707123/179590694-9a607d06-7793-455f-a12f-bf6219f61bc2.png">


### Assignment 5 interprocess communication
Assignment 5 is a server and client program where programs talk to each other through ports on the local machine. It starts with the encoding server which is given a port to work with and should be run in the background. The enc client is then used to communicate with the enc server. The enc client should be given a plaintext file, a key file, and the port of the server it needs to communicate with. After the client communicates with the server and encrypts the plaintext, this will be printed out. Next the dec server must be set up in the same was as the enc server but with a different port. Finally, the dec client will be used in a similar way to the enc client where it is given a ciphertext file, a key file, and the port to talk with the dec server on. This project directory also includes a compile all script and a clean Ex script which will compile all the needed files, and remove all the executables respectively. <br/>
<img width="547" alt="Screen Shot 2022-07-18 at 12 19 47 PM" src="https://user-images.githubusercontent.com/41707123/179600527-2d7fc721-33ed-4cd0-b552-726edc3148bf.png">


### Assignment 6 multi-threading in rust
Assignment 6 is a multi-threaded program in rust which reads in a number of partitions from the user, creates a thread for each partition that generates random numbers and collects the sum for each partition adding all the sums at the end for the total sum.<br/>
<img width="338" alt="Screen Shot 2022-07-18 at 12 46 18 PM" src="https://user-images.githubusercontent.com/41707123/179604780-7999b854-7c60-4408-a01c-3c2040ca5ff8.png">
