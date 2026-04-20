//
// Created by adna on 4/19/26.
//

#include "fs_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

void create_and_chmod(const char *filepath, mode_t permissions, const char *initial_data) {
    int fd = open(filepath, O_CREAT | O_WRONLY | O_APPEND, 0666);
    if (fd == -1) {
        perror("Error creating file");
        return;
    }

    if (initial_data != NULL) {
        if (write(fd, initial_data, strlen(initial_data)) == -1) {
            perror("Error writing initial data");
        }
    }

    if (fchmod(fd, permissions) == -1) {
        perror("Error setting permissions");
    }

    close(fd);
}

void init_district(const char *district_id) {
    char path[512];

    // 1. Create District Directory (750: rwxr-x---)
    if (mkdir(district_id, 0777) == -1 && errno != EEXIST) {
        perror("Error creating district directory");
        return;
    }
    chmod(district_id, 0750);

    // 2. Create reports.dat (664: rw-rw-r--)
    snprintf(path, sizeof(path), "%s/reports.dat", district_id);
    create_and_chmod(path, 0664, NULL);

    // 3. Create district.cfg (640: rw-r-----) with default threshold "3"
    snprintf(path, sizeof(path), "%s/district.cfg", district_id);
    struct stat st;
    if (stat(path, &st) == -1) {
        create_and_chmod(path, 0640, "3\n");
    } else {
        chmod(path, 0640);
    }

    // 4. Create logged_district (644: rw-r--r--)
    snprintf(path, sizeof(path), "%s/logged_district", district_id);
    create_and_chmod(path, 0644, NULL);

    // 5. Setup the symlink
    setup_symlink(district_id);
}

void mode_to_string(mode_t mode, char *str) {
    str[0] = (mode & S_IRUSR) ? 'r' : '-';
    str[1] = (mode & S_IWUSR) ? 'w' : '-';
    str[2] = (mode & S_IXUSR) ? 'x' : '-';
    str[3] = (mode & S_IRGRP) ? 'r' : '-';
    str[4] = (mode & S_IWGRP) ? 'w' : '-';
    str[5] = (mode & S_IXGRP) ? 'x' : '-';
    str[6] = (mode & S_IROTH) ? 'r' : '-';
    str[7] = (mode & S_IWOTH) ? 'w' : '-';
    str[8] = (mode & S_IXOTH) ? 'x' : '-';
    str[9] = '\0';
}

void setup_symlink(const char *district_id) {
    char target[512];
    char linkpath[512];

    snprintf(target, sizeof(target), "%s/reports.dat", district_id);
    snprintf(linkpath, sizeof(linkpath), "active_reports-%s", district_id);

    if (symlink(target, linkpath) == -1) {
        if (errno != EEXIST) {
            perror("Failed to create symlink");
        }
    }
}

void check_symlink_health(const char *district_id) {
    char linkpath[512];
    struct stat link_stat, target_stat;

    snprintf(linkpath, sizeof(linkpath), "active_reports-%s", district_id);

    // Check if the link itself exists
    if (lstat(linkpath, &link_stat) == -1) {
        return;
    }

    if (S_ISLNK(link_stat.st_mode)) {
        // Check if the target file exists
        if (stat(linkpath, &target_stat) == -1) {
            printf("WARNING: Symlink '%s' is dangling (points to a missing file).\n", linkpath);
        }
    }
}