#define _XOPEN_SOURCE 600
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <limits.h>
#include <ftw.h>
#include <dirent.h>
#include <sys/inotify.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_LEN (1024 * EVENT_SIZE) /* 1024 events */

int inotify_fd;
char *wd_and_paths[sizeof(int)];

static void print_event_message(struct inotify_event *event) {
	uint32_t event_mask = event->mask;

	if (event->len == 0) {
		return;
	}

	printf("[%s/%s] ", wd_and_paths[event->wd], event->name);

	if (event_mask & IN_ACCESS)
		printf("was accessed (read)");
	if (event_mask & IN_ATTRIB)
		printf("metadata changed");
	if (event_mask & IN_CLOSE_NOWRITE)
		printf("opened for read-only was closed");
	if (event_mask & IN_CLOSE_WRITE)
		printf("opened for writing was closed");
	if (event_mask & IN_CREATE)
		printf("created in watched directory");
	if (event_mask & IN_DELETE)
		printf("deleted from watched directory");
	if (event_mask & IN_DELETE_SELF)
		printf("watched file/directory was itself deleted");
	if (event_mask & IN_IGNORED)
		printf("watch was removed by kernel");
	if (event_mask & IN_MODIFY)
		printf("file was modified");
	if (event_mask & IN_MOVE_SELF)
		printf("watched file/directory was itself moved");
	if (event_mask & IN_MOVED_FROM)
		printf("moved out of watched directory");
	if (event_mask & IN_MOVED_TO)
		printf("moved into watched directory");
	if (event_mask & IN_OPEN)
		printf("was opened");
	if (event_mask & IN_UNMOUNT)
		printf("filesystem was unmounted");

	if (event->cookie > 0)
		printf(" [cookie=%4d]", event->cookie);

	printf("\n");
}

static int add_watch(const char *pathname, const struct stat *sbuf,
                     int type, struct FTW *ftwb) {
	int wd;
	if (type == FTW_D) {
		wd = inotify_add_watch(inotify_fd, pathname, IN_ALL_EVENTS);

		if (wd == -1) {
			perror("add_watch error");
			return 0;
		}

		char buf[PATH_MAX];
		realpath(pathname, buf);
		wd_and_paths[wd] = buf;

		printf("Watching directory [%s]\n", buf);
	}
	return 0;
}

void usage() {
	printf("usage: fswatch [directory] \n");
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
	ssize_t num_read;
	DIR *root;
	char buf[BUF_LEN];
	char *p;
	struct inotify_event *event;

	if (argc != 2) {
		usage();
	}

	root = opendir(argv[1]);
	if (root == NULL) {
		usage();
	}
	closedir(root);

	inotify_fd = inotify_init();
	if (inotify_fd == -1) {
		perror("oops");
		exit(EXIT_FAILURE);
	}

	nftw(argv[1], add_watch, 10, FTW_CHDIR|FTW_PHYS);
	printf("\n\n");

	for (;;) {
		num_read = read(inotify_fd, buf, BUF_LEN);
		if (num_read == -1 || num_read == 0) {
			perror("oops");
			exit(EXIT_FAILURE);
		}

		for (p = buf; p < buf + num_read;) {
			event = (struct inotify_event *) p;
			p += sizeof(struct inotify_event) + event->len;
			print_event_message(event);
		}
	}

	return EXIT_SUCCESS;
}
