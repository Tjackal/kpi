#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

sem_t	*g_forks;
sem_t	g_io;

#define N 5
#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

void	*philosopher(void *arg)
{
	int		t1;
	int		t2;
	
	sem_wait(&g_io);
	printf("*phil %d\n", *(int *)arg);
	sem_post(&g_io);
	t1 = min(*(int *)arg, (*(int *)arg + 1) % N);
	t2 = max(*(int *)arg, (*(int *)arg + 1) % N);
	while (1)
	{
		sleep((rand() % 3) + 3);
		if (sem_trywait(&g_forks[t1]) == 0)		//если получилось взять левую вилку
		{
			sem_wait(&g_io);
			printf("phil %d takes the left fork\n", *(int *)arg);
			sem_post(&g_io);
				
			if (sem_trywait(&g_forks[t2]) == 0)	//если получилось взять правую вилку
			{
				sem_wait(&g_io);
				printf("phil %d takes the right fork\n", *(int *)arg);
				sem_post(&g_io);
				
				sleep((rand() % 3) + 2);	//кушать
				
				sem_wait(&g_io);
				printf("phil %d finish the meal\t\t%d\n", *(int *)arg, *(int *)arg);
				sem_post(&g_io);
				
				sem_post(&g_forks[t2]);
				
				sem_wait(&g_io);
				printf("phil %d put the right fork\n", *(int *)arg);
				sem_post(&g_io);
			}
			sem_post(&g_forks[t1]);
			
			sem_wait(&g_io);
			printf("phil %d put the left fork\n", *(int *)arg);
			sem_post(&g_io);
		}
	}
	free(arg);
}

void	*create_number(int i)
{
	int		*n;
	
	if (!(n = (int *)malloc(sizeof(int))))
		return (NULL);
	*n = i;
	return (n);
}

int		main(void)
{
	pthread_t	*philosophers_threads;
	int			i;
	
	sem_init(&g_io, 0, 1);
	g_forks = (sem_t *)malloc(sizeof(sem_t) * N);
	i = -1;
	while (++i < N)
		sem_init(&g_forks[i], 0, 1);
	philosophers_threads = (pthread_t *)malloc(sizeof(pthread_t) * N);
	i = -1;
	while (++i < N)
		pthread_create(&philosophers_threads[i], NULL, philosopher, create_number(i));
	i = -1;
	while (++i < N)
		pthread_join(philosophers_threads[i], NULL);
	free(philosophers_threads);
	i = -1;
	while (++i < N)
		sem_destroy(&g_forks[i]);
	free(g_forks);
	return (0);
}
