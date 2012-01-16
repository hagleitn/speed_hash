#include <stdio.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

#define USERNAME_LENGTH 11
#define STEP 1
#define TRUE 1
#define FALSE 0

static int fd;

void get_names(char *fname, char **names, unsigned long *size) {
	struct stat info;

	fd = open(fname, O_RDONLY);
	if (fd == -1) {
		perror("open: ");
		exit(1);
	}

	if (fstat(fd, &info) == -1) {
		perror("fstat: ");
		exit(1);
	}

	*size = info.st_size;
	printf("file size: %d\n",(int)*size);

	void *map = mmap(NULL, *size, PROT_READ, MAP_PRIVATE, fd, 0);
	if (map == MAP_FAILED) {
		perror("mmap:");
		exit(1);
	}

	*names = (char *) map;
}

int count_names(char *names, long size) {
	int count = 0;
	while (size-- != 0) {
		if (*names++ == '\n') {
			++count;
		}
	}
	return count;
}

void swap(char **array, int pos, int pos2) {
	char *tmp = array[pos];
	array[pos] = array[pos2];
	array[pos2] = tmp;
}

void quicksort(char **array, int left, int right, int length) {
	if (left >= right) {
		return;
	}

	int pivot = left + (right-left)/2;
	swap(array, pivot, right);
	
	int idx = left;
	int i;

	for(i = left; i < right; ++i) {
		if (strncmp(array[i], array[right], length) < 0) {
			swap(array, i, idx++);
		}
	}

	swap(array, idx, right);

	quicksort(array, left, idx-1, length);
	quicksort(array, idx+1, right, length);
}

void build_sorted_array(char *names, long bytes, char ***array, int *array_size) {
	int size = count_names(names, bytes);

	*array_size = size;
	*array = (char **) malloc(*array_size*sizeof(char *));

	printf("size: %d slots\n", *array_size);
	printf("memory alloc complete\n");
	
	int current = 0;
	char *start = names;

	while (bytes-- != 0) {
		if (*names++ == '\n') {
			int position = current;
			(*array)[current++] = start;

			/* insert sort 
			while (position != 0) {
				if (strncmp((*array)[position-1], (*array)[position], names-start-1) <= 0) {
					break;
				}
				swap(*array, position, position-1);
				--position;
			}
			*/
			
			start = names;
		}
	}
	printf("quicksort\n");
	quicksort(*array, 0, *array_size-1, USERNAME_LENGTH-1);
}

int binary_search(char **array, int size, char *name, int length) {
	int left = 0, right = size-1;

	while (left <= right) {
		int middle = left + (right-left)/2;
		int cmp = strncmp(array[middle],name,length);
		if (cmp == 0) {
			return TRUE;
		} else if (cmp < 0) {
			left = middle+1;
		} else {
			right = middle-1;
		}
	}
	printf("couldn't find: %.10s\n",name);
	return FALSE;
}

void validate_names(char *names, long bytes, char **array, int array_size) {
	struct timeval  tv1, tv2;
	char *start;
	long result = 1;

    gettimeofday(&tv1, NULL);

	while ((bytes -= USERNAME_LENGTH) >= 0) {
		start = names;
		names += USERNAME_LENGTH;

		int tmp = binary_search(array, array_size, start, USERNAME_LENGTH-1);
		result = result && tmp; // false iff we didn't find at least 1
	}

	gettimeofday(&tv2, NULL);

	if (!result) {
		printf("TEST FAILED\n");
		exit(1);
	}

	printf ("Total time = %f seconds\n",
			(double) (tv2.tv_usec - tv1.tv_usec)/1000000 +
			(double) (tv2.tv_sec - tv1.tv_sec));
}

int main(int argc, char **argv) {
	char **array;
	char *names;
	unsigned long size;
	int array_size;

	printf("get_names\n");
	get_names(argv[1], &names, &size);

	printf("build_array\n");
	build_sorted_array(names, size, &array, &array_size);

	printf("validate_names\n");
	validate_names(names, size, array, array_size);
	
	free(array);
	munmap(names, size);
	close(fd);
}
