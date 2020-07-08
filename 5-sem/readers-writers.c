#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/stat.h>
#include <sys/shm.h>

#define READERS	3
#define WRITERS	3
#define ITER	20

#define RR	3	//ждущие читатели
#define SB	2	//бинарный
#define WR	1	//ждущие писатели
#define RI	0	//активные читатели

int pid = 0;
int childs = 0;
int shm = -1, sem = -1;
char* addr = (char*) -1; // укзатель на память, икрементируемую писателями в CR

struct sembuf start_reader[5]	= {{RR, 1, 0}, //увеличиваем семафор ждущих чит.
            {SB,  0, 0}, //ждем освобождения ресурса
            {WR,  0, 0}, //ждем пока отработают писатели
            {RI,  1, 0}, //увеличиваем семафор активных чит.
            {RR, -1, 0}}, //уменьшаем семафор ждущих чит.
    stop_reader[1]	= {{RI, -1, 0}}, //уменьшаем семафор активных чит.
    start_writer_w[1]	= {{WR,  1, 0}}, //увеличиваем семафор ждущих пис.
    start_writer_s[3] = {{RI,  0, 0}, //ждем пока отработают читатели,
              {SB,  -1, 0}, //ждем освобождения ресурса
              {WR, -1, 0}}, //захватываем ресурс
    stop_writer[1]  = {{SB,  1, 0}};

void terminate (int param)
{
	printf ("\033[0m");
	printf ("\nEnd\n");
	
	int status;
	while(childs--)
		wait(&status);

	// удаление семафора
	if (sem != -1)
		semctl(sem, 0, IPC_RMID);
	else 
		exit(param);
	
	// отключение разделяемой памяти
	if (shm != -1)	
		shmdt(addr);		
	else
		exit(param);
	
	// удаление разделяемой памяти
	if (addr != (char*) -1)
		shmctl(shm, 0, IPC_RMID);
	else
		exit(param);

	exit(param);
}

void startRead(int sem) { 
	semop(sem, start_reader, 5); 
}

void stopRead(int sem) { 
	semop(sem, stop_reader, 1);
	//semop(sem,one_up,1);
}

void startWrite(int sem) {
	//semop(sem,one_up,1);
	semop(sem, start_writer_w, 1);
	semop(sem, start_writer_s, 3);
}

void stopWrite(int sem) { 
	semop(sem, stop_writer, 1); 
}

// Функция читателей
void reader(int id, int sem, int* addr)
{
	while (1)
	{			
		startRead(sem);
		
		int t = (*addr);
		printf("Reader # %d read %d\n", id, t);
		
		stopRead(sem);

		if(t > ITER)
			exit(0);
		sleep(1);
	}
	exit(0);
}

// Функция писателей
void writer(int id, int sem, int* addr)
{
	while (1) 
	{
		startWrite(sem);
		
		int t = ++(*addr);
		printf("Writer # %d wrote %d\n", id, t);
		
		stopWrite(sem);

		if(t > ITER)
			exit(0);
		sleep(2);
	}
	exit(0);
}

int main(int argc, char* argv[]) {	
	int perms = IPC_CREAT | S_IRWXU | S_IRWXG | S_IRWXO;
	
	// Семафоры
	if ((sem = semget(IPC_PRIVATE, 4, perms)) == -1)
	{
		perror("semget"); 
		terminate(0);
	}
	semctl(sem, 0, SETVAL, 0);
	semctl(sem, 1, SETVAL, 0);
	semctl(sem, 2, SETVAL, 0);
	semctl(sem, 3, SETVAL, 0);

	// Разделяемая память
	if ((shm = shmget(IPC_PRIVATE, sizeof(char), perms)) == -1)
	{ 
		perror("shmget"); 
		terminate(1);
	}
	
	if ((addr = (char*)shmat(shm, 0, 0)) == (char*)-1)
	{ 
		perror("shmat"); 
		terminate(1); 
	}
	
	*addr = 0;
	
	semop(sem,one_up,1);
	// Создаём писателей
	for (int i = 0; i < WRITERS; i++)
	{
		if ((pid = fork()) != -1)
		{
			if(!pid)
				writer(i+1, sem, (int*)addr);
			else 
				childs++;
		}
		else
		{ 
			perror("fork"); 
			terminate(1); 
		}
	}
	
	// Создаём читателей
	for (int i=0; i < READERS; i++)
	{
		if ((pid = fork()) != -1)
		{
			if(!pid)
				reader(i+1, sem, (int*)addr);
			else 
				childs++;
		}
		else
		{ 
			perror("fork"); 
			terminate(1); 
		}
	}
	
	// Установим свои обработчики сигналов
	signal(SIGABRT, &terminate);
	signal(SIGTERM, &terminate);
	signal(SIGINT, &terminate);	
	signal(SIGKILL, &terminate);

	int status;
	for(int i = 0; i < childs; i++)
		wait(&status);
	terminate(0);
}
