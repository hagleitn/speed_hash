#include <stdio.h>
#include <sys/mman.h>
#include <sys/time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

#define USERNAME_LENGTH 10
#define STEP 1
#define FALSE 0
#define TRUE 1

static char *min_name;
static char *max_name;

static int fd;
static int words = 0;

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

	*names = malloc(*size);
	int res = read(fd, *names, *size);
	if (res != *size) {
		perror("read:");
		exit(1);
	}

	int i;
	for (i = 0; i < *size; ++i) {
		if ((*names)[i] == '\n') {
			(*names)[i] = '\0';
			++words;
		}
	}

	min_name = (char *)*names;
	max_name = (char *)*names+*size;
}

int count_names(char *names, long size) {
	return words;
}

inline unsigned long hash(unsigned char *str) {
	unsigned long hash = 5381;
	int c;

	while ((c = *str++) != '\0') {
		hash = ((hash << 5) + hash) + c;
	}

	return hash;
}

typedef struct _bucket {
	char *name;
	struct _bucket *next;
} bucket;

inline int check(char *name, char **table, int hash_size) {
	int key = hash(name) % hash_size;
	
	if (table[key] == NULL) {
		printf("empty: %s\n", name);
		return FALSE;
	} else {
		if (min_name <= table[key] && max_name > table[key]) {
			return strcmp(name, table[key]) == 0;
		}
		
		bucket *b = (bucket *)table[key];
		while(b != NULL) {
			if (strcmp(name, b->name) == 0) {
				return TRUE;
			}
			b = b->next;
		}
		printf("empty again: %s\n", name);
		return FALSE;
	}
}

inline void insert(char *name, char **table, int hash_size, int *collisions) {
	int key = hash(name) % hash_size;
	
	if (table[key] == NULL) {
		table[key] = name;
	} else {
		++(*collisions);
		if (min_name <= table[key] && max_name > table[key]) {
			bucket *b = (bucket*) malloc(sizeof(bucket));
			b->name = table[key];
			b->next = NULL;
			table[key] = (char *)b;
		}
		
		bucket *b = (bucket*) table[key];
		while(b->next != NULL) {
			b = b->next;
		}
		
		bucket *tmp = (bucket*) malloc(sizeof(bucket));
		tmp->name = name;
		tmp->next = NULL;

		b->next = tmp;
	}
}

void report_collisions(char **table, int hash_size) {
	FILE *f = fopen("report.txt","w");
	int i;

	for (i = 0; i < hash_size; ++i) {
		int count = 0;
		if (table[i] != NULL) {
			if (min_name <= table[i] && max_name > table[i]) {
				count = 1;
			} else {
				bucket *b = (bucket*) table[i];
				while(b != NULL) {
					b = b->next;
					count++;
				}
			}
		}
		fprintf(f,"%d\n",count);
	}
	fclose(f);
}
		

void build_hash(char *names, long bytes, char ***table, int *hash_size) {
	int size = count_names(names, bytes);
	int collisions = 0;
	int count = 0;

	*hash_size = size;

	printf("hash size: %d\n", *hash_size);

	*table = (char **) malloc(*hash_size*sizeof(char *));
	memset(*table,0,*hash_size*sizeof(char *));

	printf("memory alloc complete\n");
	
	char *start = names;
	while (bytes-- != 0) {
		if (*names++ == '\0') {
			insert(start, *table, *hash_size, &collisions);
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
	int key;
	long result = TRUE;

    gettimeofday(&tv1, NULL);

	while ((bytes -= USERNAME_LENGTH+1) >= 0) {
		result = result && check(names, table, hash_size);
		names += USERNAME_LENGTH+1;
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
	
	report_collisions(table, hash_size);
	
	printf("validate_names\n");
	validate_names(names, size, table, hash_size);

	free(table);
	close(fd);
}
