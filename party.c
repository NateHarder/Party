#include <semaphore.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

int num_students;
int num_taxis;
int max_time;
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
    printf("Student %d: I am at the party. It is way more fun than what I expected...", num);
    sleep(generate_random());
    printf("Student %d: I am done partying . I better get back to that calculus homework that is due tomorrow...", num);
    sem_wait(&in_taxi);
    get_in(num);
    sem_post(&stud_lock);
    pthread_exit(0);
}

void taxi_task(int num) {
    sem_wait(&taxi_lock);
    printf("Taxi %d: I arrived at the curb...There is no one that wants to go home...I might as well take a nap..", num);
    sem_wait(&stud_lock);
    printf("Taxi %d: I have one student %d... When will I find the other passengers? Sigh. The students seem to have too much fun these days. ", num, taxi_list[0]);
    sem_wait(&stud_lock);
    printf("Taxi %d: I have two  %d, %d When will I find the other passengers? Sigh. The students seem to have too much fun these days", num, taxi_list[0], taxi_list[1]);
    sem_wait(&stud_lock);
    printf("Taxi 1 : I have three students %d,%d, %d When will I find the other passengers? Sigh. The students seem to have too much fun these days", num, taxi_list[0], taxi_list[1], taxi_list[2]);
    sem_wait(&stud_lock);
    printf("Taxi 1 : I have all four... 1,5,6,7 Time to drive....BYE", num, taxi_list[0], taxi_list[1], taxi_list[2], taxi_list[3]);
    sem_post(&taxi_lock);
    pthread_exit(0);
}



int main(int argc, char **argv) {
    if (argc != 7) {
        printf("Invalid arguments. Use format [./party –s 12 –t 4 –m 10].");
    } else {
        num_students = atoi(argv[2]);
        num_taxis = atoi(argv[4]);
        max_time = atoi(argv[6]);
    }

    pthread_t students[num_students];
    pthread_t taxis[num_taxis];
    sem_init(&in_taxi, 0, 4);
    sem_init(&stud_lock, 0, 0);
    sem_init(&taxi_lock, 0, 1);

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
