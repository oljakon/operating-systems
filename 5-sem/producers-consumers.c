#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static const unsigned int PRODUCERS = 3;
static const unsigned int CONSUMERS = 3;
static const int COUNT = 3;
static const int N = 9;
static const int PERM = S_IRWXU | S_IRWXG | S_IRWXO;
    // S_IRWXU - владелец имеет права на чтение, запись и выполнение файла
    // S_IRWXG - группа имеет права на чтение, запись и выполнение файла
    // S_IRWXO все остальные (вне группы) имеют права на чтение, запись и выполнение файла

static int* shared_buffer;
static int sh_pos_cons;
static int sh_pos_prod;

#define SB 0
#define SE 1 // количество пустых ячеек
#define SF 2 // количество заполненных ячеек

#define P -1
#define V 1

// четыре массива структур по два семафора
static struct sembuf producer_start[2] = {{SE, P, 0}, {SB, P, 0}};
static struct sembuf producer_stop[2] =  {{SB, V, 0}, {SF, V, 0}};
static struct sembuf consumer_start[2] = {{SB, P, 0}, {SF, P, 0}};
static struct sembuf consumer_stop[2] =  {{SB, V, 0}, {SE, V, 0}};

void producer(const int semid, const int value)
{
    sleep(rand() % 5);
    int sem_op_p = semop(semid, producer_start, 2);
    if (sem_op_p == -1)
    {
        perror("Can't make operation on semaphors. Stop.\n");
        exit(1);
    }

    shared_buffer[sh_pos_prod] = value;
    printf("Producer № %d: element %d\n", getpid(), value);
    (sh_pos_prod)++;

    int sem_op_v = semop(semid, producer_stop, 2);
    if (sem_op_v == -1)
    {
        perror("Can't make operation on semaphors. Stop.\n");
        exit(1);
    }
}

void consumer(const int semid) //, const int value)
{
    sleep(rand() % 2);
    int sem_op_p = semop(semid, consumer_start, 2);
    if (sem_op_p == -1)
    {
        perror("Can't make operation on semaphors. Stop.\n");
        exit( 1 );
    }

    printf("Consumer № %d: element %d\n", getpid(), shared_buffer[sh_pos_cons]);
    (sh_pos_cons)++;

    int sem_op_v = semop(semid, consumer_stop, 2);
    if (sem_op_v == -1)
    {
        perror("Can't make operation on semaphors. Stop.\n");
        exit(1);
    }
}

int main()
{
    int shmid, semid;

    int parent_pid = getpid();
    printf("Parent pid: %d\n", parent_pid);

    if ((shmid = shmget(IPC_PRIVATE, (N + 1) * sizeof(int), IPC_CREAT | PERM)) == -1)
    {
        perror("Unable to create a shared area. Stop.\n");
        exit(1);
    }

        shared_buffer = shmat(shmid, 0, 0);
        if (*shared_buffer == -1)
        {
            perror("Can't attach memory. Stop.\n");
            exit(1);
        }

        (sh_pos_prod) = 0;
        (sh_pos_cons) = 0;

    if ((semid = semget(IPC_PRIVATE, 3, IPC_CREAT | PERM)) == -1)
    {
        perror("Unable to create a semaphore. Stop.\n");
        exit(1);
    }

    int ctrl_sb = semctl(semid, SB, SETVAL, 1);
    int ctrl_se = semctl(semid, SE, SETVAL, N);
    int ctrl_sf = semctl(semid, SF, SETVAL, 0);

    if ( ctrl_se == -1 || ctrl_sf == -1 || ctrl_sb == -1)
    {
        perror("Can't set control semaphors. Stop.\n");
        exit(1);
    }

    pid_t producers[PRODUCERS];
    // создаем производителей
    for (unsigned int i = 0; i < PRODUCERS; ++i)
    {
        if (getpid() == parent_pid)
        {
            producers[i] = fork();
            if (producers[i] == -1)
            {
                perror("Can't create new process. Stop.\n");
                exit(1);
            }
        }
    }

    pid_t consumers[CONSUMERS];
    // создаем потребителей
    for (unsigned int i = 0; i < CONSUMERS; ++i)
    {
        if (getpid() == parent_pid)
        {
            consumers[i] = fork();
            if (consumers[i] == -1)
            {
                perror("Can't create new process. Stop.\n");
                exit(1);
            }
        }
    }

    for (int i = 0; i < PRODUCERS; ++i)
    {
        if (producers[i] == 0)
        {
            int value = i * 10;
            int stop = COUNT + value;
            while (value < stop)
            {
                producer(semid, value);
                value++;
            }
            return 0;
        }
    }
    for (int i = 0; i < CONSUMERS; ++i)
    {
        if (consumers[i] == 0)
        {
            unsigned int value = 0;
            while (value < (COUNT * PRODUCERS) / CONSUMERS)
            {
                consumer(semid);
                value++;
            }
        }
    }
    if (getpid() == parent_pid)
    {
        int status;
        pid_t ret_value;
        while ((ret_value = wait(&status)) != -1)
        {
        }

        if (shmdt(shared_buffer) == -1)
        {
            perror("Can't detach shared memory. Stop.\n");
            exit(1);
        }
    }
    return 0;
}
