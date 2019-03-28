#include <pthread.h> 
#include <semaphore.h> 
#include <stdio.h> 
#include <stdlib.h>

#define N 5
#define Iterations 500

#define RED   "\x1B[31m"
#define GRN   "\x1B[32m"
#define END   "\x1B[0m"

//threads for philosophers
pthread_t philosopher_thread_id[N];
pthread_t status_id;
sem_t chopstick[N];


int Max_hunger=0;

//Philosopher model
struct philosopher{
    int id;
    int status;
    int eat_interval;
    int think_interval;
    int time_remaining;
    int hunger_level;
    int eat_count;
}philosophers[N];
const char* s[] = {"THINKING", "HUNGRY", "-EATING-","DEAD"};

//Think & Eat count down
void sleep_thread(int t,int philo_id){
    philosophers[philo_id].time_remaining = t;
    int i;
    for(i=0;i<t;i++){
        philosophers[philo_id].time_remaining--;
        sleep(1);
    }
}
//Hunger Count Up
void* hunger_thread(void* i_address){
    int i = *(int*) i_address;
    philosophers[i].hunger_level=0;
    while(1){

    if(++philosophers[i].hunger_level >Max_hunger) Max_hunger = philosophers[i].hunger_level;
    sleep(1);
    }
}

//Philosoper thread function
void* philosopher_run(void* i_address){
    int i = *(int*) i_address;
   while(1){ 
 
    //THINK
    philosophers[i].status = 0;
    sleep_thread(philosophers[i].think_interval,i);
    //HUNGRY
    pthread_t hunger_id;
    philosophers[i].status = 1;
    //LOCK
    pthread_create(&hunger_id,NULL,hunger_thread,&philosophers[i]);
    sem_wait(&chopstick[i]);
    sem_wait(&chopstick[(i+1) % N]);
    pthread_cancel(hunger_id);

    //EAT
    philosophers[i].status = 2;
    philosophers[i].eat_count++;
    philosophers[i].hunger_level = 0;
    sleep_thread(philosophers[i].eat_interval,i);
    //RELEASE
    sem_post(&chopstick[i]);
    sem_post(&chopstick[(i+1) % N]); 
       
   }

}



//Status Thread 
void* status_run(void* null){
    //Philosophers in Thinking State: Pi(time left in thinking state)
    int i;
    for(i=0;i<Iterations+1;i++)
    {
        printf("\n\e[4m--interation - %d ----Max-hunger %d\e[0m\n",i,Max_hunger);
        int I=0;
        for(I=0;I<N;I++){
            printf("philosopher %d had %d meals & is in %s%s%s count %d \n",
            I+1,
            philosophers[I].eat_count,
            (philosophers[I].status == 1) ?RED:GRN,
            s[philosophers[I].status],
            END,
            (philosophers[I].status == 0 || philosophers[I].status == 2) ? philosophers[I].time_remaining:philosophers[I].hunger_level
            );
        }
       sleep(1);
    }
    
    //Exit all philosopher threads
    for (i = 0; i < N; i++)
        pthread_cancel(philosopher_thread_id[i]); 
    
}

int main(){

    int i;
    //create chopsitcks
    for (i = 0; i < N; i++) 
        sem_init(&chopstick[i], 0, 1); 

    
    //create philosophers & threads
    for (i = 0; i < N; i++){
        struct philosopher p;
        p.id = i;
        p.status = 1;
        p.eat_interval = rand() % (5 + 1 - 0) + 1;
        p.think_interval = rand() % (5 + 1 - 0) + 1;;
        p.time_remaining = p.think_interval;
        p.eat_count=0;
        p.hunger_level=0;
        philosophers[i] = p;
        
        pthread_create(&philosopher_thread_id[i],NULL,philosopher_run,&philosophers[i]);
    }
    //Create status thread
    pthread_create(&status_id,NULL,status_run,NULL);

   //wait for threads to finish
    for (i = 0; i < N; i++)
        pthread_join(philosopher_thread_id[i], NULL); 
    pthread_join(status_id, NULL); 
    
    return 0;
}