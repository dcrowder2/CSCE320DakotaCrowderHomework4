#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <errno.h>
#include <string.h>
#include <signal.h>
/*
A struct used to define all the parameters for the threads that are the philosophers
*/
typedef struct {
  int position;
  char* name;
} params_t;
//Pre defining the methods
void setTable(int forksCount);
void sitPhilosophers();
void getMutex(int philosopherPosition);
void getForks(int philosopherPosition);
void getKnife(int philosopherPosition);
void *philosopher(void *params);
void think(char* name);
void dine(char* name);
void safeQuit(int sig);
char** philosopherNameDatabase;
int TIME_MAX;
pthread_t *philosophers;
int philosopherCount;
int knivesCount;
sem_t *mutex;
sem_t *forks;
sem_t *knives;

void createNameDatabase()
{
  philosopherNameDatabase = malloc(sizeof(char) * 75);
  philosopherNameDatabase[0] = "Plato";
  philosopherNameDatabase[1] = "Nietzche";
  philosopherNameDatabase[2] = "Aristotle";
  philosopherNameDatabase[3] = "Descartes";
  philosopherNameDatabase[4] = "Kant";
  philosopherNameDatabase[5] = "Boole";
  philosopherNameDatabase[6] = "Marx";
  philosopherNameDatabase[7] = "Chomsky";
  philosopherNameDatabase[8] = "Locke";
  philosopherNameDatabase[9] = "Voltaire";
  philosopherNameDatabase[10] = "Machiavelli";
}
int main(int argc, char *argv[])
{
  signal(SIGINT, safeQuit);//catch both ctrl+c and ctrl+z so the program can stop correctly
  signal(SIGTSTP, safeQuit);
  createNameDatabase();
  if(argc != 4)
  {
    perror("Please enter in the correct parameters\"dphil <N> <0/K> <TIME> \" <N> : number of philosophers, <0/1> : 0 – version (a) of the dining philosophers, K – the knife version with the problem where K is a number of knives available in the shared knife pool. <TIME> : the random amount of time for a philosopher to sleep or eat between 0-TIME seconds.");
  }
  else
  {
      philosopherCount = atoi(argv[1]);
      knivesCount = atoi(argv[2]);
      TIME_MAX = atoi(argv[3]);
      philosophers = malloc(sizeof(pthread_t) * philosopherCount);
      mutex = malloc(sizeof(sem_t));
      forks = malloc(sizeof(sem_t) * philosopherCount);
      if(knivesCount >0)
      {
        knives = malloc(sizeof(sem_t) * knivesCount);
      }
      setTable(philosopherCount);
      sitPhilosophers();
      while(1)
      {
        //just here to occupy the parent
      }
  }
}

void safeQuit(int sig)//done
{
  printf("Killing threads...\n");
  for(int i = 0; i < philosopherCount; i ++)
  {
    pthread_kill(philosophers[i], SIGABRT);
  }

  free(philosophers);
  free(philosopherNameDatabase);
  printf("Destroying semaphores...\n");
  if(sem_destroy(mutex) <0)
  {
    perror("Error destroying mutex.");
    exit(0);
  }
  if(sem_destroy(forks) <0)
  {
    perror("Error destroying forks.");
    exit(0);
  }
  if(sem_destroy(knives) <0)
  {
    perror("Error destroying knives.");
    exit(0);
  }
  printf("All clean, exiting...\n");
  exit(0);
}

void setTable(int forksCount)
{
  for(int i = 0; i < forksCount; i++)
  {
    sem_init(&forks[i], 0, 1);
  }
  for(int i = 0; i < knivesCount; i++)
  {
    sem_init(&knives[i], 0, 1);
  }
  sem_init(mutex, 0, 5);
}

void sitPhilosophers()
{
  int i;
  for(i = 0; i < philosopherCount; i++)
  {
    params_t *arg = malloc(sizeof(params_t));
    arg->position = i;
    if(i < 11)//there are only 11 predefined names for philosophers
    {
      arg->name = malloc(sizeof(philosopherNameDatabase[i]));
      arg->name = philosopherNameDatabase[i];
    }
    else
    {
      arg->name = malloc(sizeof(philosopherNameDatabase[i])+3);
      arg->name = philosopherNameDatabase[i];
      strcat(arg->name, "(1)"); //adding (1) at the end of a repeat, making the maximum number of philosophers be 22 instead of 11
    }
    pthread_create(&philosophers[i], NULL, philosopher, (void *)arg);
  }
}

void threadSignalHandler(int sig)
{
  signal(sig, SIG_IGN);
  pthread_exit(0);
}

void *philosopher(void *params)
{
  int succesfulEatCount = 0;
  signal(SIGABRT, threadSignalHandler);
  params_t myInfo = *(params_t *)params;
  while(1)
  {
    printf(myInfo.name);
    printf(": I have eaten %u times\n", succesfulEatCount);
    think(myInfo.name);
    printf(myInfo.name);
    printf(": I am hungry\n");
    sem_wait(mutex);
    sem_wait(&forks[myInfo.position]);
    sem_wait(&forks[(myInfo.position + 1) % philosopherCount]);
    if(knivesCount > 0)
    {
      int i;
      for(i = 0; i < knivesCount; i++)
      {
        if(sem_trywait(&knives[i]) == 0)
        {
          dine(myInfo.name);
          sem_post(&knives[i]);
          succesfulEatCount++;
          i = knivesCount+1;
        }
      }
      if(i == knivesCount)
      {
        srand(time(NULL));
        i = rand() % knivesCount;
        sem_wait(&knives[i]);
        dine(myInfo.name);
        sem_post(&knives[i]);
        succesfulEatCount++;
      }

    }
    else
    {
      dine(myInfo.name);
      succesfulEatCount++;
    }
    sem_post(&forks[(myInfo.position + 1) % philosopherCount]);
    sem_post(&forks[myInfo.position]);
    sem_post(mutex);

  }
}

void think(char* name)
{
  printf(name);
  printf(": I am thinking...\n");
  srand(time(NULL));
  int timetosleep = rand() % TIME_MAX;
  sleep(timetosleep);
}

void dine(char* name)
{
  printf(name);
  printf(": I am eating...\n");
  srand(time(NULL));
  int timetosleep = rand() % TIME_MAX;
  sleep(timetosleep);
  printf(name);
  printf(": done eating\n");
}
