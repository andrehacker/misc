#include <iostream>
#include <vector>
#include <thread>
#include "SafeQueue.cpp"

const int CAPACITY = 100;
const int WRITES_PER_THREAD = 100;
const int THREAD_COUNT = 10;

SafeQueue<int> queue(CAPACITY);

void write_to_queue() {
	for (int i=0; i<WRITES_PER_THREAD; i++) {
		queue.push_front(i);
	}
	// push negative number at end, will terminate one reader
	queue.push_front(-1);
}

std::mutex coutMutex;
int read_from_queue(int thread_id) {
	int count_read = 0;
	int item;
	do {
		queue.pop_back(&item);
		++count_read;
	} while (item >= 0);
	
	{	// explicite scope, so that lock get's deleted!
		std::lock_guard<std::mutex> lock(coutMutex);
		
		std::cout << "Reader " << thread_id << " consumed " << count_read << " items." << std::endl;
	}
}

int main() {
	std::cout << "Start" << std::endl;
	
	std::vector<std::thread> writer_threads, reader_threads;
	
	// Start writer and consumer threads
	for (int i = 0; i < THREAD_COUNT; i++) {
		writer_threads.push_back(std::thread(write_to_queue));
		reader_threads.push_back(std::thread(read_from_queue, i));
	}
	
	// join
	// DOES NOT WORK due to definition in thread header: thread(thread&) = delete;
	//for(std::thread t : writer_threads) t.join();	// DOES NOT WORK???
	for (auto t = writer_threads.begin(); t != writer_threads.end(); ++t) {
		t->join();	// dereferencing allows to access value of current iterator item
	}
	for (auto t = reader_threads.begin(); t != reader_threads.end(); ++t) {
		t->join();
	}
	std::cout << "Full waits: " << queue.get_full_count() << " Empty waits:" << queue.get_empty_count() << std::endl;
	
	std::cout << "END" << std::endl;
	
}
