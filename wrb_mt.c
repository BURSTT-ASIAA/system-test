/*
$ gcc -o wrb_mt wrb_mt.c -pthread
clear memory cache before run the program, then remove destination file if exists, then run the program, 8 threads in the following example
source file: /mnt/fpga2, destination file: /mnt/data/fpga2.mt
$ echo 3 | sudo tee /proc/sys/vm/drop_caches ; sudo rm /mnt/data/fpga2.mt ; time sudo taskset -c 35-50 wrb_mt -pthread 8
 */
#include <stdio.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <pthread.h>
#include <string.h>
#include <stdlib.h>

#define MAP_HUGE_1GB    (30 << MAP_HUGE_SHIFT)

struct ThreadData {
    char* source_data;
    char* destination_data;
    off_t offset;
    off_t size;
};

void* thread_copy(void* arg) {
    struct ThreadData* data = (struct ThreadData*)arg;
    off_t offset = data->offset;
    off_t size = data->size;

    // Copy the data from source to destination
    memcpy(data->destination_data + offset, data->source_data + offset, size);

    pthread_exit(NULL);
}

int main(int argc, char *argv[]) {
    	const char* source_path = "/mnt/fpga2";
    	const char* destination_path = "/mnt/data/fpga2.mt";
    	int source_file, destination_file;
    	struct stat st;
    	off_t file_size;
    	char* source_data;
    	char* destination_data;

	unsigned int NUM_THREADS;
	unsigned int i;
        for(i=1;i<argc; i++) {
                if(strcmp(argv[i], "-pthread")==0) NUM_THREADS=atoi(argv[i+1]);

        }

    	pthread_t threads[NUM_THREADS];
    	struct ThreadData thread_data[NUM_THREADS];	

    	// Open the source file
    	source_file = open(source_path, O_RDONLY);
    	if (source_file == -1) {
        	perror("Failed to open source file");
        	return 1;
    	}

    	// Get the size of the source file
    	if (fstat(source_file, &st) == -1) {
        	perror("Failed to get file size");
        	close(source_file);
        	return 1;
    	}
    	file_size = st.st_size;

    	// Open the destination file
    	destination_file = open(destination_path, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
    	if (destination_file == -1) {
        	perror("Failed to open destination file");
        	close(source_file);
        	return 1;
    	}

    	// Resize the destination file to match the source file size
    	if (ftruncate(destination_file, file_size) == -1) {
        	perror("Failed to resize destination file");
        	close(source_file);
        	close(destination_file);
        	return 1;
    	}

    	// Create memory maps of the source and destination files
    	source_data = mmap(NULL, file_size, PROT_READ, MAP_SHARED | MAP_HUGETLB | MAP_HUGE_1GB, source_file, 0);
    	destination_data = mmap(NULL, file_size, PROT_WRITE, MAP_SHARED | MAP_HUGE_1GB, destination_file, 0);
    	if (source_data == MAP_FAILED || destination_data == MAP_FAILED) {
        	perror("Failed to create memory map");
        	close(source_file);
        	close(destination_file);
        	return 1;
    	}

    	// Calculate the chunk size for each thread
    	off_t chunk_size = file_size / NUM_THREADS;

    	// Create threads and copy file data in parallel
    	for (int i = 0; i < NUM_THREADS; i++) {
        	off_t offset = i * chunk_size;
        	off_t size = (i == NUM_THREADS - 1) ? (file_size - offset) : chunk_size;

        	thread_data[i].source_data = source_data;
        	thread_data[i].destination_data = destination_data;
        	thread_data[i].offset = offset;
        	thread_data[i].size = size;

        	if (pthread_create(&threads[i], NULL, thread_copy, &thread_data[i]) != 0) {
            		fprintf(stderr, "Failed to create thread %d\n", i);
            	break;
        	}
    	}

    	// Wait for all threads to complete
    	for (int i = 0; i < NUM_THREADS; i++) {
        	pthread_join(threads[i], NULL);
    	}

    	// Unmap
	if (munmap(source_data, file_size) == -1 || munmap(destination_data, file_size) == -1) {
        	perror("Failed to unmap memory");
    	}

    	// Close the files
    	close(source_file);
    	close(destination_file);

    	printf("File copied successfully.\n");

    	return 0;
}
