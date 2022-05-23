#include "lib.h"
#include "types.h"
#define N 5

void test_scanf();
void test_sem();
void test_philosopher();
void test_producer_consumer();
void test_reader_writer();
void philosopher(int i, sem_t *forks);
void producer(int id, sem_t *mutex, sem_t *full, sem_t *empty);
void consumer(int id, sem_t *mutex, sem_t *full, sem_t *empty);
void writer(sem_t *writemutex);
void reader(sem_t *writemutex, sem_t *countmutex);
int rcount = 0;
int uEntry(void) {

	// test_scanf();
	
	test_sem();

	// For lab4.3
	// TODO: You need to design and test the philosopher problem.
	// Note that you can create your own functions.
	// Requirements are demonstrated in the guide.
	
	//哲学家
	// test_philosopher();
	//生产者消费者问题
    // test_producer_consumer();
	//读者写者问题
	// test_reader_writer();

	exit(0);
	return 0;
}

void test_scanf()
{
	// 测试scanf	
	int dec = 0;
	int hex = 0;
	char str[6];
	char cha = 0;
	int ret = 0;
	while(1){
		printf("Input:\" Test %%c Test %%6s %%d %%x\"\n");
		ret = scanf(" Test %c Test %6s %d %x", &cha, str, &dec, &hex);
		printf("Ret: %d; %c, %s, %d, %x.\n", ret, cha, str, dec, hex);
		if (ret == 4)
			break;
	}
}

void test_sem()
{
	// 测试信号量
	int i = 4;
	sem_t sem;
	printf("Father Process: Semaphore Initializing.\n");
	int ret = sem_init(&sem, 0);
	if (ret == -1) {
		printf("Father Process: Semaphore Initializing Failed.\n");
		exit();
	}

	ret = fork();
	if (ret == 0) {
		while( i != 0) {
			i --;
			printf("Child Process: Semaphore Waiting.\n");
			sem_wait(&sem);
			printf("Child Process: In Critical Area.\n");
		}
		printf("Child Process: Semaphore Destroying.\n");
		sem_destroy(&sem);
		exit();
	}
	else if (ret != -1) {
		while( i != 0) {
			i --;
			printf("Father Process: Sleeping.\n");
			sleep(128);
			printf("Father Process: Semaphore Posting.\n");
			sem_post(&sem);
		}
		printf("Father Process: Semaphore Destroying.\n");
		sem_destroy(&sem);
		exit();
	}
}

void test_philosopher()
{
	sem_t forks[N];
	int id;
	for (int i = 0; i < N; ++i)
		sem_init(&forks[i], 1);
	for (int i = 0; i < N - 1; ++i)
	{
		int ret = fork();
		if (ret == -1)
		{
			printf("fork error: %d\n", i);
			exit();
		}
		if (ret == 0) // child
		{
			id = get_pid();
			philosopher(id, forks);
		}
	}
	id = get_pid();
	philosopher(id, forks);
}

void test_producer_consumer()
{
	sem_t mutex, full, empty;
	sem_init(&mutex, 1);
	sem_init(&full, 0);
	sem_init(&empty, N);
	int id;
	for (int i = 0; i < N - 2; ++i)
	{
		int ret = fork();
		if (ret == -1)
		{
			printf("fork error: %d\n", i);
			exit();
		}
		if (ret == 0) // child
		{
			id = get_pid();
			producer(id, &mutex, &full, &empty);
		}
	}
	fork();
	id = get_pid();
	consumer(id, &mutex, &full, &empty);
}

void test_reader_writer()
{
	sem_t writemutex, countmutex;
	sem_init(&writemutex, 1);
	sem_init(&countmutex, 1);
	exec(0);
	int ret = 1;
	for (int i = 0; i < 5; ++i)
		if (ret > 0)
		{
			ret = fork();
			// printf("fork return %d in loop %d\n", ret, i);
		}
	int id = get_pid();
	if (id < 4)
	{
		// printf("reader %d\n", id - 1);
		reader(&writemutex, &countmutex);
	}
	else if (id < 7)
	{
		// printf("writer %d\n", id - 4);
		writer(&writemutex);
	}
}

void philosopher(int i, sem_t *forks)
{
	while (1)
	{
		if (i % 2 == 0)
		{
			sem_wait(&forks[i]);
			sleep(64);
			sem_wait(&forks[(i + 1) % N]);
		}
		else
		{
			sem_wait(&forks[(i + 1) % N]);
			sleep(64);
			sem_wait(&forks[i]);
		}
		printf("Philosopher %d: eat\n", i);
		sleep(64); // eat
		printf("Philosopher %d: think\n", i);
		sem_post(&forks[i]);
		sleep(64);
		sem_post(&forks[(i + 1) % N]);
		sleep(64); // think
	}
}

void producer(int id, sem_t *mutex, sem_t *full, sem_t *empty)
{
	--id; // pid start at 1
	while (1)
	{
		sem_wait(empty);
		sleep(64);
		sem_wait(mutex);
		sleep(64);
		printf("Producer %d\n", id);
		sleep(64);
		sem_post(mutex);
		sleep(64);
		sem_post(full);
		sleep(64);
	}
}

void consumer(int id, sem_t *mutex, sem_t *full, sem_t *empty)
{
	if(id == 5) id =2;
	while (1)
	{
		sem_wait(full);
		sleep(64);
		sem_wait(mutex);
		sleep(64);
		printf("Consumer %d\n",id);
		sleep(64);
		sem_post(mutex);
		sleep(64);
		sem_post(empty);
		sleep(64);
	}
}

void writer(sem_t *writemutex)
{
	int id = get_pid() - 4;
	while (1)
	{
		sem_wait(writemutex);
		printf("Writer %d: write\n", id);
		sleep(64);
		sem_post(writemutex);
		sleep(64);
	}
}

void reader(sem_t *writemutex, sem_t *countmutex)
{
	int id = get_pid() - 1;
	int rcount;
	while (1)
	{
		sem_wait(countmutex);
		sleep(64);
		rcount = exec(-1);
		if (rcount == 0)
			sem_wait(writemutex);
		sleep(64);
		rcount = exec(-1);
		++rcount;
		exec(rcount);
		sleep(64);
		sem_post(countmutex);
		printf("Reader %d: read, total %d reader\n", id, rcount);
		sleep(64);
		sem_wait(countmutex);
		rcount = exec(-1);
		--rcount;
		exec(rcount);
		sleep(64);
		rcount = exec(-1);
		if (rcount == 0)
			sem_post(writemutex);
		sleep(64);
		sem_post(countmutex);
		sleep(64);
	}
}



