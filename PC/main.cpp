#include <iostream>
#include <cstdlib>
#include <vector>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>

using namespace std;

constexpr auto BUFFER_SIZE = 512;

char buffer[BUFFER_SIZE];
bool valid[BUFFER_SIZE];
pthread_mutex_t mutex;
sem_t empty2, full;

struct Task
{
	int index;
	bool isProducer;
	int start;
	int end;
	char product_id;
};

vector<Task> tasks;

char try_insert_item(int index)
{
	bool success = false;
	char item = tasks[index].product_id;

	cout << "Task " << tasks[index].index << " is going to sleep for start." << endl;
	sleep(tasks[index].start);
	
	sem_wait(&empty2);
	pthread_mutex_lock(&mutex);

	cout << "Task " << tasks[index].index << " is going to sleep for end." << endl;
	sleep(tasks[index].end);

	cout << "Task " << tasks[index].index << " is now inserting item." << endl;

	for (int i = 0; i < BUFFER_SIZE; i++)
	{
		if (!valid[i])
		{
			valid[i] = true;
			buffer[i] = item;
			success = true;
			break;
		}
	}

	pthread_mutex_unlock(&mutex);
	sem_post(&full);

	return success ? 'O' : 'X';
}

char try_remove_item(int index)
{
	char ans;

	cout << "Task " << tasks[index].index << " is going to sleep for start." << endl;
	sleep(tasks[index].start);

	sem_wait(&full);
	pthread_mutex_lock(&mutex);

	cout << "Task " << tasks[index].index << " is going to sleep for end." << endl;
	sleep(tasks[index].end);

	cout << "Task " << tasks[index].index << " is now removing item." << endl;

	for (int i = 0; i < BUFFER_SIZE; i++)
	{
		if (valid[i])
		{
			valid[i] = false;
			ans = buffer[i];
			break;
		}
	}

	pthread_mutex_unlock(&mutex);
	sem_post(&empty2);

	return ans;
}

void* deal_task(void* param)
{
	int index = *(int*)param;
	char* s = new char();
	if (tasks[index].isProducer)
	{
		*s = try_insert_item(index);
	}
	else
	{
		*s = try_remove_item(index);
	}
	return s;
}

int main()
{
	int index, start, end;
	char product_id;
	int cnt = 0;
	char role;
	int* const_num;
	void** return_values;

	while (cin >> index)
	{
		cin >> role >> start >> end;
		if (role == 'P')
		{
			cin >> product_id;
		}
		tasks.push_back({ index, role == 'P', start, end, product_id });
		cnt++;
	}

	const_num = new int[cnt];
	return_values = new void*[cnt];

	vector<pthread_t> tids(cnt);
	pthread_attr_t attr;

	pthread_attr_init(&attr);

	pthread_mutex_init(&mutex, NULL);
	sem_init(&empty2, PTHREAD_PROCESS_PRIVATE, 5);
	sem_init(&full, PTHREAD_PROCESS_PRIVATE, 0);

	for (int i = 0; i < cnt; i++)
	{
		const_num[i] = i;
		pthread_create(&tids[i], &attr, deal_task, const_num + i);
	}

	for (int i = 0; i < cnt; i++)
	{
		pthread_join(tids[i], return_values + i);
	}

	for (int i = 0; i < cnt; i++)
	{
		cout << "Task " << tasks[i].index << " returns value " << *(char*)(return_values[i]) << endl;
		delete (char*)(return_values[i]);
	}

	delete[] const_num;
	delete[] return_values;
	
	return 0;
}