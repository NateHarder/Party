#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <stdint.h>

/*
Authors: Nathaniel Harder, Brendan Embrey
Assignment Number: 3
Date of Submission: 11/25/20
Name of this file: party.c
Description of the program: party.c takes arguments from the commandline for number of students, number of taxis, and maximum wait time. Then, it simulates the party problem with semaphores and threads by having each student (simulated by a thread) spend a random amount of time less than the maximum time at the party before getting in a taxi (simulated by a thread) to leave. Each taxi only has four seats, and students must wait to find an empty seat. Each taxi must wait for the taxi before it to leave, as well.

The function get_in simulates a student entering the taxi by placing the student number in the taxi_list array. The function generate_random generates a random number to be used as a student's party time. They were grouped together because they are minor functions that facilitate the use of the other functions. 

The function student_task simulates a student spending time at a party then enetring a taxi. The function taxi_task simulates a taxi waiting for four students then leaving. They were grouped together as they are the two functions which are used by the taxi and student threads.

The function handle_args ensures that all of the arguments are given correctly. The function main passes arguments to handle_args and creates and waits for all of the student and taxi threads to finish. They were grouped together as they handle input from the user and begin the running of the program.

*/

/* The number of students at the party. */
int num_students;
/* The number of taxis. */
int num_taxis;
/*  The maximum time spent at the party. */
int max_time;
/* The seats left in the taxi at the curb */
int seats_left;
/* Array list of all the students in the taxi at the curve. */
int taxi_list[4] = {-1,-1,-1,-1};

/* Semaphore used to block taxi threads until a new student leaves the party. */
sem_t stud_lock;
/* Semaphore used to block taxi threads from arriving all at once. */
sem_t taxi_lock;
/* Semaphore used to block student threads from entering a taxi if the taxi_list is full. */
sem_t in_taxi;
/* Semaphore used to keep multiple students from decrementing the seats left. */
sem_t seat_lock;
/* Semaphore used to keep multiple students from getting in a taxi.  */
sem_t enter_lock;

/*
Function Name: get_in
Input to the method: Student number integer.
Output(Return value): None. 
Brief description of the task: Locks other students from getting in, finds the next empty seat (simulated by a -1 value) in the taxi_array and replaces that value with the student number to simulate that student sitting in a seat.
*/
void get_in(int num) {
    /* Block other students. */
    sem_wait(&enter_lock);
    /* If one of the seats is empty, replace its value with the student number then break. */
    for(int i = 0; i < 4; i++) {
        if (taxi_list[i] == -1) {
            taxi_list[i] = num;
            break;
        }
    }
     /* Unblock other students. */
    sem_post(&enter_lock);
}

/*
Function Name: generate_random
Input to the method: None.
Output(Return value): Randomized integer. 
Brief description of the task: Uses rand() function to find a random number less than  or equal to max_time then returns it to be used to determine how long a student is at the party.
*/
int generate_random() {
    /* Take the value of rand() mod max_time+1 to get a number between 0 and max_time, then return it. */
    int num = rand() % (max_time + 1);
    return num;
}

/*
Function Name: student_task
Input to the method: Student number integer.
Output(Return value): None. 
Brief description of the task: Prints a message saying the student has arrived, waits for a random amount of time, prints a message saing the sudent is leaving, then either waits for an empty seat or gets in a taxi if a seat is available.
*/
void student_task(int num) {
    /* Print arrival message, sleep for a random time, then prints departure message. */
    printf("Student %d: I am at the party. It is way more fun than what I expected...\n\n", num);
    sleep(generate_random());
    printf("Student %d: I am done partying . I better get back to that calculus homework that is due tomorrow...\n\n", num);
    /* If no seats are left wait for the next taxi, otherwise decrement the number of seats. */
    sem_wait(&seat_lock);
    if (seats_left == 0) {
        sem_wait(&in_taxi);
    }
    seats_left--;
    /* When a seat is available, get in, then unlock the stud_lock so the taxi will do its next task, then exit the thread. */
    get_in(num);
    //sleep(generate_random());
    sem_post(&seat_lock);
    sem_post(&stud_lock);
    pthread_exit(0);
}

/*
Function Name: taxi_task
Input to the method: Taxi number integer.
Output(Return value): None. 
Brief description of the task: Decrements the taxi_lock so other taxis wait, then prints an arrival message, and waits for stud_lock to be incremented to print the next message saying a new student has gotten in. Then, when four students are in, print a departure message, clear the taxi_list array of open seats, then post the in_taxi semaphore so the next student gets in a new taxi, and posts the taxi_lock semaphore so a new taxi arrives, then exits the thread. 
*/
void taxi_task(int num) {
    /* Make other taxi threads wait, print arrival message, then wait for a student thread to finish and post stud_lock before continuing. */
    sem_wait(&taxi_lock);
    printf("Taxi %d: I arrived at the curb...There is no one that wants to go home...I might as well take a nap..\n\n", num);
    sem_wait(&stud_lock);
    /* When a student has arrived, print a message then wait for the next student to post stud_lock. */
    printf("Taxi %d: I have one student %d... When will I find the other passengers? Sigh. The students seem to have too much fun these days.\n\n", num, taxi_list[0]);
    sem_wait(&stud_lock);
    /* Repeat the process for the second student. */
    printf("Taxi %d: I have two  %d, %d When will I find the other passengers? Sigh. The students seem to have too much fun these days\n\n", num, taxi_list[0], taxi_list[1]);
    sem_wait(&stud_lock);
    /* Repeat the process for the third student. */
    printf("Taxi %d: I have three students %d,%d, %d When will I find the other passengers? Sigh. The students seem to have too much fun these days\n\n", num, taxi_list[0], taxi_list[1], taxi_list[2]);
    sem_wait(&stud_lock);
    /* When the fourth student arrives, print a departure message, return the number of seats left to four, and clear the taxi_list array of students to simulate the next taxi arriving. */
    //sleep(generate_random());
    printf("Taxi %d: I have all four... %d,%d,%d,%d Time to drive....BYE\n\n", num, taxi_list[0], taxi_list[1], taxi_list[2], taxi_list[3]);
    seats_left = 4;
    for(int i = 0; i < 4; i++) {
        taxi_list[i] = -1;
    }
    /* Increment in_taxi and taxi_lock so that the next taxi arrives and the next student can get in, then exit the thread. */
    sem_post(&in_taxi);
    sem_post(&taxi_lock);
    pthread_exit(0);
}


/*
Function Name: handle_args
Input to the method: Command line argument array.
Output(Return value): None. 
Brief description of the task: Handles the given arguments and checks to make sure they fit in the specified format. If they do, set the number of students and taxis and the max time limit to the values given. If they don't print an error message and exit.
*/
void handle_args(int argc, char **argv) {
    /* If not enough arguments are given print error then exit. */
    if (argc != 7) {
        printf("Invalid arguments. Use format [./party –s [number of students] –t [number of taxis] –m [max party time]].\n");
        exit(0);
    } else {
        /* If the arguments are given in the correct format set all values to the values of the given arguments. */        
        int correct_format = strcmp(argv[1], "-s") == 0 && strcmp(argv[3], "-t") == 0 && strcmp(argv[5], "-m") == 0;
        if (correct_format == 1) {
            num_students = atoi(argv[2]);
            num_taxis = atoi(argv[4]);
            max_time = atoi(argv[6]);
            /* If any of the chosen arguments is invalid, print an error and exit. */
            if (num_students == 0 || num_taxis == 0 || max_time == 0) {
                printf("Arguments must be integers above 0.\n");
                exit(0);
            }
        /* If the arguments aren't in the right format, print an error and exit. */
        } else {
            printf("Invalid arguments. The input arguments must use the argument specifiers -s, -t, and -m, each followed by a positive integer to represent the number of students, number of taxis, and the maximum party time.\n");
            exit(0);
        }
        /* Set seats_left to four and if there aren't four students for each taxi, print an error and exit. */
        seats_left = 4;
        if (num_students != num_taxis*4) {
            printf("Number of students must be 4 times number of taxis.");
            exit(0);
        }
    }
}

/*
Function Name: main
Input to the method: Command line argument array.
Output(Return value): Zero if successful. 
Brief description of the task: Checks to make sure argument string is supplied correctly then sets num_students, num_taxis, and max_time to the numbers supplied. Then initializes arrays for student and taxi threads, initializes semaphores, and creates and joins each student and taxi thread so the program waits for them all to execute. 
*/
int main(int argc, char **argv) {
    /* Handle command line arguments. */
    handle_args(argc, argv);
    /* Initialize student and taxi thread arrays then initialize semaphores. */
    pthread_t students[num_students];
    pthread_t taxis[num_taxis];
    sem_init(&stud_lock, 0, 0);
    sem_init(&taxi_lock, 0, 1);
    sem_init(&in_taxi, 0, 0);
    sem_init(&seat_lock, 0, 1);
    sem_init(&enter_lock,0, 1);


    /* Create each taxi thread in the array and pass the taxi number. */
    for (int i = 0; i < num_taxis; i++) {
        pthread_create(&taxis[i], NULL, (void *) (*taxi_task), (void *) (uintptr_t) i);
    }

    /* Create each student thread in the array and pass the student number. */
    for (int i = 0; i < num_students; i++) {
        pthread_create(&students[i], NULL, (void *) (*student_task), (void *) (uintptr_t) i);
    }

    /* Join to each taxi thread and wait for each to exit. */
    for (int i = 0; i < num_taxis; i++) {
        pthread_join(taxis[i], NULL);
    }

    /* Join to each student thread and wait for each to exit. */
    for (int i = 0; i < num_students; i++) {
        pthread_join(students[i], NULL);
    }

}
