#include <windows.h>
#include <time.h>
#include <stdlib.h>
#include <stdio.h>

CRITICAL_SECTION	g_io;
CRITICAL_SECTION	g_number_free_seats_lock;
CRITICAL_SECTION	g_index_lock;
CRITICAL_SECTION	g_current_client_lock;
CONDITION_VARIABLE	*g_seat;
CONDITION_VARIABLE	g_barber;
CONDITION_VARIABLE	g_current_client;
int					g_number_free_seats;
int					g_number_seats;
int					g_nclients;
int					g_i = -1;;
int					g_current_index;

DWORD WINAPI		barber(void *arg)
{
	EnterCriticalSection(&g_io);
	printf("*barber %d\n", *(int *)arg);
	LeaveCriticalSection(&g_io);
	while (1)
	{
		EnterCriticalSection(&g_number_free_seats_lock);
		if (g_number_free_seats == g_number_seats)
			SleepConditionVariableCS(&g_barber, &g_number_free_seats_lock, INFINITE);
		LeaveCriticalSection(&g_number_free_seats_lock);

		EnterCriticalSection(&g_io);
		printf("barber call client\n");
		LeaveCriticalSection(&g_io);

		WakeConditionVariable(&g_seat[g_current_index]);
		Sleep(1000 * (rand() % 3));
		WakeConditionVariable(&g_current_client);

		EnterCriticalSection(&g_io);
		printf("barber finish client\n");
		LeaveCriticalSection(&g_io);
	}
	return (0);
}

DWORD WINAPI		client(void *arg)
{
	while (1)
	{
		Sleep(1000 * ((rand() % 5) + 2));
		EnterCriticalSection(&g_io);
		printf("*client %d\n", *(int *)arg);
		LeaveCriticalSection(&g_io);
		EnterCriticalSection(&g_number_free_seats_lock);
		if (g_number_free_seats > 0)
		{
			if (g_number_free_seats == g_number_seats)
				WakeConditionVariable(&g_barber);
			g_number_free_seats--;

			EnterCriticalSection(&g_io);
			printf("client %d take seat %d\n", *(int *)arg, g_number_seats - g_number_free_seats);
			LeaveCriticalSection(&g_io);
			LeaveCriticalSection(&g_number_free_seats_lock);

			EnterCriticalSection(&g_index_lock);
			g_i = (g_i + 1) % (g_number_seats - 1);
			SleepConditionVariableCS(&g_seat[g_i], &g_index_lock, INFINITE);
			g_current_index = (g_current_index + 1) % (g_number_seats - 1);

			EnterCriticalSection(&g_io);
			printf("client %d go to barber %d\n", *(int *)arg, g_number_free_seats);
			LeaveCriticalSection(&g_io);
			LeaveCriticalSection(&g_index_lock);

			EnterCriticalSection(&g_number_free_seats_lock);
			g_number_free_seats++;
			LeaveCriticalSection(&g_number_free_seats_lock);

			EnterCriticalSection(&g_current_client_lock);
			SleepConditionVariableCS(&g_current_client, &g_current_client_lock, INFINITE);
			EnterCriticalSection(&g_io);
			printf("client %d leave barber %d\n", *(int *)arg, g_number_free_seats);
			LeaveCriticalSection(&g_io);
			LeaveCriticalSection(&g_current_client_lock);
		}
		else
		{
			LeaveCriticalSection(&g_number_free_seats_lock);

			EnterCriticalSection(&g_io);
			printf("client %d go away %d\n", *(int *)arg, g_number_free_seats);
			LeaveCriticalSection(&g_io);
		}
	}
	return (0);
}

void				read_input(int argc,char  **argv, int *g_nclients, int *g_nseats, int *g_nfree_seats)
{
	if (argc == 2)
	{
		*g_nclients = atoi(argv[1]);
		*g_nseats = atoi(argv[2]);
	}
	else
	{
		*g_nclients = 3;
		*g_nseats = 2;
	}
	*g_nfree_seats = *g_nseats;
}

int					main(int argc, char **argv)
{
	HANDLE	h_barber;
	HANDLE	*h_clients;
	int		i;

	read_input(argc, argv, &g_nclients, &g_number_seats, &g_number_free_seats);
	InitializeCriticalSection(&g_io);
	InitializeCriticalSection(&g_number_free_seats_lock);
	InitializeCriticalSection(&g_index_lock);
	InitializeCriticalSection(&g_current_client_lock);
	InitializeConditionVariable(&g_barber);
	InitializeConditionVariable(&g_current_client);
	g_seat = (CONDITION_VARIABLE *)malloc(sizeof(CONDITION_VARIABLE) * g_number_seats);
	i = -1;
	while (++i < g_number_seats)
		InitializeConditionVariable(&g_seat[i]);
	if (!(h_barber = CreateThread(NULL, 0, barber, &h_barber, 0, NULL)))
		return (-1);
	if (!(h_clients = (HANDLE *)malloc(sizeof(HANDLE) * g_nclients)))
		return (-2);
	i = -1;
	while (++i < g_nclients)
		if (!(h_clients[i] = CreateThread(NULL, 0, client, &h_clients[i], 0, NULL)))
			return (-1);
	WaitForMultipleObjects(g_nclients, h_clients, TRUE, INFINITE);
	i = -1;
	while (++i < g_nclients)
		CloseHandle(h_clients[i]);
	CloseHandle(h_barber);
	DeleteCriticalSection(&g_io);
	DeleteCriticalSection(&g_number_free_seats_lock);
	DeleteCriticalSection(&g_index_lock);
	DeleteCriticalSection(&g_current_client_lock);
	free(g_seat);
    return (0);
}
