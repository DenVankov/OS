#include <stdio.h>
#include <unistd.h>

void parent(int pid, int fd) {
	int a = 5;
	printf("Parent: %d\n", pid);
	write(fd, &a, sizeof(a));
}

void child(int pid, int fd) {
	int a = 4;
	printf("Child: %d\n", pid);
	read(fd, &a, sizeof(a));
	printf("%d\n", a);
}

int main(void) {
	int fd[2];
	int err = pipe(fd);
	int pid = fork();

	if (!err) {
		if (pid > 0)
			parent(pid, fd[1]);
		else
			child(pid, fd[0]);
	}

	return err;
}
