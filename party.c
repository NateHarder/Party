#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int num_students;
int num_taxis;
int max_time;
int seats_left;
int taxi_list[4] = {-1,-1,-1,-1};

sem_t stud_lock;
sem_t taxi_lock;
sem_t in_taxi;

void get_in(int num) {
    for(int i = 0; i < 4; i++) {
        if (taxi_list[i] == -1) {
            taxi_list[i] = num;
            break;
        }
    }
}

int generate_random() {
    int num = rand() % (max_time + 1);
    return num;
}

void student_task(int num) {
    printf("Student %d: I am at the party. It is way more fun than what I expected...\n\n", num);
    sleep(generate_random());
    printf("Student %d: I am done partying . I better get back to that calculus homework that is due tomorrow...\n\n", num);
    if (num_in_taxi == 0) {
        sem_wait(&in_taxi);
    } else {
        seats_left--;
    }
    get_in(num);
    //sleep(generate_random());
    sem_post(&stud_lock);
    pthread_exit(0);
}

void taxi_task(int num) {
    sem_wait(&taxi_lock);
    printf("Taxi %d: I arrived at the curb...There is no one that wants to go home...I might as well take a nap..\n\n", num);
    sem_wait(&stud_lock);
    printf("Taxi %d: I have one student %d... When will I find the other passengers? Sigh. The students seem to have too much fun these days.\n\n", num, taxi_list[0]);
    sem_wait(&stud_lock);
    printf("Taxi %d: I have two  %d, %d When will I find the other passengers? Sigh. The students seem to have too much fun these days\n\n", num, taxi_list[0], taxi_list[1]);
    sem_wait(&stud_lock);
    //sleep(generate_random());
    printf("Taxi %d: I have three students %d,%d, %d When will I find the other passengers? Sigh. The students seem to have too much fun these days\n\n", num, taxi_list[0], taxi_list[1], taxi_list[2]);
    sem_wait(&stud_lock);
    printf("Taxi %d: I have all four... %d,%d,%d,%d Time to drive....BYE\n\n", num, taxi_list[0], taxi_list[1], taxi_list[2], taxi_list[3]);
    seats_left = 4;
    for(int i = 0; i < 4; i++) {
        taxi_list[i] = -1;
    }
    sem_post(&in_taxi);
    sem_post(&taxi_lock);
    pthread_exit(0);
}

int main(int argc, char **argv) {
    if (argc != 7) {
        printf("Invalid arguments. Use format [./party –s [number of students] –t [number of taxis] –m [max party time].");
        exit(0);
    } else {
        num_students = atoi(argv[2]);
        num_taxis = atoi(argv[4]);
        max_time = atoi(argv[6]);
        seats_left = 4;
        if (num_students != num_taxis*4) {
            printf("Number of students must be 4 times number of taxis.");
            exit(0);
        }
    }

    pthread_t students[num_students];
    pthread_t taxis[num_taxis];
    sem_init(&stud_lock, 0, 0);
    sem_init(&taxi_lock, 0, 1);
    sem_init(&in_taxi, 0, 0);

    for (int i = 0; i < num_students; i++) {
        pthread_create(&students [i], NULL, student_task, (void *) i);
    }

    for (int i = 0; i < num_taxis; i++) {
        pthread_create(&taxis [i],NULL, taxi_task, (void *) i);
    }

    for (int i = 0; i < num_students; i++) {
        pthread_join(students[i], NULL);
    }

    for (int i = 0; i < num_taxis; i++) {
        pthread_join(taxis[i], NULL);
    }
}
