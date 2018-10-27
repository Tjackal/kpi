#include <iostream>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdlib.h>
#include <time.h>

#define N 4
#define M 4
#define K 10

struct queue_node
{
	int data;
	queue_node *next;
};

queue_node *begin = NULL, *end = NULL;

void add_node()
{
	if (end == NULL)
	{
		begin = new queue_node;
		begin->data = 0;
		begin->next = NULL;
		end = begin;
	}
	else
	{
		queue_node *new_node = new queue_node;
		new_node->data = 0;
		new_node->next = NULL;
		end->next = new_node;
		end = new_node;
	}

	return;
}

void delete_node()
{
	if (begin == NULL)
		return;

	queue_node *first_node = begin;
	begin = begin->next;
	delete first_node;
	return;
}

sem_t sem_products_to_add, sem_products_to_take, sem_block_begin, sem_block_end, sem_block_count;
int count = 0;

void* producer(void*)
{
	int r, i = 0;
	while (i < 10)
	{
		r = 6;
	
		sem_wait(&sem_products_to_add);
		sem_wait(&sem_block_count);	
		sem_wait(&sem_block_end);
		if (count == 1)
		{
			sem_wait(&sem_block_begin);
			++count;
			std::cout << "Product added (" << count << "). Sleeping for " << r << ".\n";
			sem_post(&sem_block_count);

			add_node();

			sem_post(&sem_block_begin);
		}
		else
		{
			++count;
			std::cout << "Product added (" << count << "). Sleeping for " << r << ".\n";
			sem_post(&sem_block_count);

			add_node();
		}
	
		//em_wait(&sem_block_count);
		//++count;
		//std::cout << count;
		//sem_post(&sem_block_count);

		//sem_post(&sem_block_end);

		
		sem_post(&sem_block_end);	
		sem_post(&sem_products_to_take);
		sleep(r);
		++i;
	}

	return NULL;
}

void* consumer(void*)
{
	int r, i = 0;
	while (i < 10)
	{
		r = 6;
		sem_wait(&sem_products_to_take);

		sem_wait(&sem_block_count);
		
		sem_wait(&sem_block_begin);
		if (count == 1)
		{
			sem_wait(&sem_block_end);
			--count;
			std::cout << "Product token (" << count << "). Sleeping for " << r << ".\n";

			sem_post(&sem_block_count);

			delete_node();

			sem_post(&sem_block_end);
		}
		else
		{
			--count;
			std::cout << "Product token (" << count << "). Sleeping for " << r << ".\n";

			sem_post(&sem_block_count);

			delete_node();
		}
		sem_post(&sem_block_begin);
		sem_wait(&sem_block_count);
		//--count;
		r = 6;
		//std::cout << count;
		sem_post(&sem_block_count);

		//sem_post(&sem_block_begin);

		sem_post(&sem_products_to_add);
		

		sleep(r);
		++i;
	}

	return NULL;
}

int main()
{
	srand(time(NULL));
	sem_init(&sem_products_to_add, 0, K);
	sem_init(&sem_products_to_take, 0, 0);
	sem_init(&sem_block_begin, 0, 1);
	sem_init(&sem_block_end, 0, 1);
	sem_init(&sem_block_count, 0, 1);
	//init threads
	pthread_t producers[N], consumers[M];

	for(int i = 0; i < M; ++i)
	{
		pthread_create(&consumers[i], NULL, &consumer, NULL);
	}

	for(int i = 0; i < N; ++i)
	{
		pthread_create(&producers[i], NULL, &producer, NULL);
	}


	sleep(7);


	for(int i = 0; i < M; ++i)
	{
		pthread_join(consumers[i], NULL);
	}

	for(int i = 0; i < N; ++i)
	{
		pthread_join(producers[i], NULL);
	}



	sem_destroy(&sem_products_to_add);
	sem_destroy(&sem_products_to_take);
	sem_destroy(&sem_block_begin);
	sem_destroy(&sem_block_end);
	sem_destroy(&sem_block_count);
    printf("asd\n");
	return 0;
}
