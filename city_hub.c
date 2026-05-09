//
// Created by adna on 5/9/26.
//
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

void handle_start_monitor() {
    pid_t hub_mon_pid = fork();

    if (hub_mon_pid == -1) {
        perror("Fork failed");
        return;
    }

    if (hub_mon_pid == 0) {
        int pipe_fd[2];
        if (pipe(pipe_fd) == -1) {
            perror("Pipe failed");
            exit(1);
        }

        pid_t monitor_pid = fork();
        if (monitor_pid == -1) {
            perror("Fork failed");
            exit(1);
        }

        if (monitor_pid == 0) {
            close(pipe_fd[0]);
            dup2(pipe_fd[1], STDOUT_FILENO);
            close(pipe_fd[1]); // closing the original descriptor since it's now duplicated

            //overwrite the process with the monitor executable
            execlp("./monitor_reports", "monitor_reports", NULL);
            perror("Exec failed");
            exit(1);
        } else {
            close(pipe_fd[1]); //hub_mon only reads

            char buffer[256];
            int bytes_read;

            while ((bytes_read = read(pipe_fd[0], buffer, sizeof(buffer) - 1)) > 0) {
                buffer[bytes_read] = '\0';
                printf(">> [Hub Received]: %s", buffer);
                fflush(stdout);
            }

            close(pipe_fd[0]);
            waitpid(monitor_pid, NULL, 0);
            printf("\n>> [Hub Notice]: The monitor process has officially ended.\n");
            exit(0); // hub_mon's job is done
        }
    }
    // city_hub returns immediately to the command prompt without waiting
}

void handle_calculate_scores(char *args) {
    char *district = strtok(args, " ");

    int pipes[10][2];
    pid_t pids[10];
    int num_districts = 0;

    while (district != NULL && num_districts < 10) {
        if (pipe(pipes[num_districts]) == -1) {
            perror("Pipe failed");
            return;
        }

        pids[num_districts] = fork();
        if (pids[num_districts] == -1) {
            perror("Fork failed");
            return;
        }

        if (pids[num_districts] == 0) {
            close(pipes[num_districts][0]);

            dup2(pipes[num_districts][1], STDOUT_FILENO);
            close(pipes[num_districts][1]); // Close the original descriptor

            execlp("./scorer", "scorer", district, NULL);
            perror("Exec failed");
            exit(1);
        } else {
            close(pipes[num_districts][1]); // parent only reads, so close writing end immediately
        }

        district = strtok(NULL, " "); // get the next district in the string
        num_districts++;
    }

    for (int i = 0; i < num_districts; i++) {
        waitpid(pids[i], NULL, 0);

        char buffer[1024];
        int bytes_read;
        while ((bytes_read = read(pipes[i][0], buffer, sizeof(buffer) - 1)) > 0) {
            buffer[bytes_read] = '\0';
            printf("%s", buffer);
        }
        close(pipes[i][0]);
    }
}

int main() {
    char input[256];

    printf("\n");
    printf("   City Hub Interactive Shell Started\n");
    printf("\n");

    while (1) {
        printf("city_hub> ");
        if (fgets(input, sizeof(input), stdin) == NULL) break;

        input[strcspn(input, "\n")] = '\0';
        if (strlen(input) == 0) continue;

        if (strcmp(input, "exit") == 0) {
            break;
        } else if (strcmp(input, "start_monitor") == 0) {
            handle_start_monitor();
        } else if (strncmp(input, "calculate_scores", 16) == 0) {
            handle_calculate_scores(input + 17);
        } else {
            printf("Unknown command. Available: start_monitor, calculate_scores, exit\n");
        }
    }

    printf("Exiting City Hub...\n");
    return 0;
}