#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <fcntl.h>
#include <string.h>

volatile sig_atomic_t keep_running = 1;

void handle_sigint(int sig) {
    (void)sig; // suppress unused warning
    keep_running = 0;
}

void handle_sigusr1(int sig) {
    (void)sig; // suppress unused warning
    // Use write instead of printf because it's async-signal-safe
    const char *msg = "New report added!\n";
    write(STDOUT_FILENO, msg, strlen(msg));
}

int main() {
    // 1. Setup signal handlers using sigaction
    struct sigaction sa_int;
    sa_int.sa_handler = handle_sigint;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    if (sigaction(SIGINT, &sa_int, NULL) == -1) {
        perror("sigaction SIGINT");
        exit(1);
    }

    struct sigaction sa_usr1;
    sa_usr1.sa_handler = handle_sigusr1;
    sigemptyset(&sa_usr1.sa_mask);
    sa_usr1.sa_flags = 0;
    if (sigaction(SIGUSR1, &sa_usr1, NULL) == -1) {
        perror("sigaction SIGUSR1");
        exit(1);
    }

    // 2. Create/Overwrite .monitor_pid
    int fd = open(".monitor_pid", O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (fd < 0) {
        perror("open .monitor_pid");
        exit(1);
    }
    char pid_str[32];
    int len = snprintf(pid_str, sizeof(pid_str), "%d\n", getpid());
    write(fd, pid_str, len);
    close(fd);

    printf("Monitor started with PID %d.\n", getpid());

    // 3. Main Loop
    while (keep_running) {
        pause(); // Wait for signals
    }

    // 4. Shutdown
    printf("\nMonitor terminating.\n");
    unlink(".monitor_pid");
    return 0;
}
