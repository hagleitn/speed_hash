#include <stdio.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

#define USERNAME_LENGTH 11
#define STEP 1
#define C1 1
#define C2 1

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

inline unsigned long hash_1(unsigned char *str, int size) {
	unsigned long hash = 5381;
	int c;

	while (size-- != 0) {
		c = *str++;
		hash = ((hash << 5) + hash) + c;
	}

	return hash;
}

inline unsigned long hash_2(unsigned char *str, int size) {
    int hash, i;
    for(hash = i = 0; i < size; ++i)
    {
        hash += str[i];
        hash += (hash << 10);
        hash ^= (hash >> 6);
    }
    hash += (hash << 3);
    hash ^= (hash >> 11);
    hash += (hash << 15);
    return hash;
}

inline unsigned long hash(unsigned long hash1, unsigned long hash2, int table_size, int position) {
  unsigned long t1 = position*C1;
  unsigned long t2 = position*position*C2;
  unsigned long hash = (hash1  + t1 + t2) % table_size;
  return hash;
}

void build_hash(char *names, long bytes, char ***table, int *hash_size) {
	int size = count_names(names, bytes);
	int collisions = 0;
	int count = 0;

	*hash_size = size + 2*size/3;

	printf("hash size: %d slots\n", *hash_size);

	*table = (char **) malloc(*hash_size*sizeof(char *));
	memset(*table,0,*hash_size*sizeof(char *));

	printf("memory alloc complete\n");
	
	char *start = names;
	while (bytes-- != 0) {
		if (*names++ == '\n') {

         int collision_count = 0;
         int hash1 = hash_1(start, names - start -1);
         int hash2 = hash_2(start, names - start -1);
         int key = hash(hash1, hash2, *hash_size, collision_count);
         while ((*table)[key] != NULL) {
				key = hash(hash1, hash2, *hash_size, ++collision_count);
            collisions++;
			}
         //printf("inserting name %s", start);

			(*table)[key] = start;
			start = names;

			if (++count % 10000000 == 0) {
				printf("inserted: %d\n",count);
			}
		}
	}
	printf("collisions: %d\n", collisions);
}

void validate_names(char *names, long bytes, char **table, int hash_size) {
	struct timeval  tv1, tv2;
	char *start;
	long result = 1;

    gettimeofday(&tv1, NULL);

	while ((bytes -= USERNAME_LENGTH+1) >= 0) {
		start = names;
      names += USERNAME_LENGTH+1;
      int collision_count = 0;
      int hash1 = hash_1(start, names - start -1);
      int hash2 = hash_2(start, names - start -1);
      int key = hash(hash1, hash2, hash_size, collision_count);
      while (table[key] != 0 && strncmp(table[key], start, USERNAME_LENGTH) != 0) {
        key = hash(hash1, hash2, hash_size, ++collision_count);
      }
      //printf("%d %d %f", count, collision_count, time);

		result = result && (long)table[key]; // false iff we didn't find at least 1
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
	char **table;
	char *names;
	unsigned long size;
	int hash_size;

	printf("get_names\n");
	get_names(argv[1], &names, &size);

	printf("build_hash\n");
	build_hash(names, size, &table, &hash_size);

	printf("validate_names\n");
	validate_names(names, size, table, hash_size);
	
	free(table);
	munmap(names, size);
	close(fd);
}
