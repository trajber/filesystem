#include <sys/stat.h>
#include <sys/types.h>
#include <stdlib.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

int main (int argc, char *argv[]) {
	if (argc != 2) {
		exit(EXIT_FAILURE);
	}

	struct stat buffer;
	int fd = open(argv[1], O_RDONLY);
	int status = fstat(fd, &buffer);
	char *file_data = malloc(buffer.st_size);

	memset(file_data, 0, buffer.st_size);

	if (read(fd, file_data, buffer.st_size - 1) == -1) {
		perror("Oops:");
		exit(EXIT_FAILURE);
	}

	printf("File size: [%zd] bytes\n", buffer.st_size);
	printf("File data was: %s\n", file_data);
	close(fd);

}



