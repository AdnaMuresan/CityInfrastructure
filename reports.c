//
// Created by adna on 4/19/26.
//

#include "reports.h"
#include "fs_utils.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/stat.h>
#include <time.h>

void add_report(const char *district_id, Report *new_report) {
    char path[512];
    snprintf(path, sizeof(path), "%s/reports.dat", district_id);

    // Open for appending
    int fd = open(path, O_WRONLY | O_APPEND);
    if (fd == -1) {
        perror("Failed to open reports.dat for adding");
        return;
    }

    if (write(fd, new_report, sizeof(Report)) != sizeof(Report)) {
        perror("Failed to write report");
    } else {
        printf("Report #%d added successfully.\n", new_report->report_id);
    }

    close(fd);
}

void list_reports(const char *district_id) {
    char path[512];
    snprintf(path, sizeof(path), "%s/reports.dat", district_id);

    struct stat st;
    if (stat(path, &st) == -1) {
        perror("Could not stat reports.dat");
        return;
    }

    // Convert permission bits to string using our helper
    char perm_str[10];
    mode_to_string(st.st_mode, perm_str);

    // Print file information header
    printf("=== District: %s ===\n", district_id);
    printf("File Size: %ld bytes | Permissions: %s | Last Modified: %s",
           (long)st.st_size, perm_str, ctime(&st.st_mtime));
    printf("----------------------------------------------------\n");

    int fd = open(path, O_RDONLY);
    if (fd == -1) return;

    Report r;
    // Read records one by one until end of file
    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        printf("ID: %d | Inspector: %s | Category: %s | Severity: %d\n",
               r.report_id, r.inspector_name, r.category, r.severity);
    }

    close(fd);
}

void view_report(const char *district_id, int report_id) {
    char path[512];
    snprintf(path, sizeof(path), "%s/reports.dat", district_id);

    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        perror("Failed to open reports.dat");
        return;
    }

    Report r;
    int found = 0;
    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        if (r.report_id == report_id) {
            printf("--- Report Details ---\n");
            printf("ID: %d\n", r.report_id);
            printf("Inspector: %s\n", r.inspector_name);
            printf("Category: %s\n", r.category);
            printf("Severity: %d\n", r.severity);
            printf("Coordinates: %f, %f\n", r.latitude, r.longitude);
            printf("Timestamp: %s", ctime(&r.timestamp));
            printf("Description: %s\n", r.description);
            found = 1;
            break;
        }
    }

    if (!found) printf("Report %d not found.\n", report_id);
    close(fd);
}

void remove_report(const char *district_id, int target_id) {
    char path[512];
    snprintf(path, sizeof(path), "%s/reports.dat", district_id);

    // O_RDWR because we need to both read next records and write them backward
    int fd = open(path, O_RDWR);
    if (fd == -1) {
        perror("Failed to open reports.dat");
        return;
    }

    Report r;
    off_t write_pos = -1;

    // 1. Find the record to delete
    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        if (r.report_id == target_id) {
            // Found it! Mark the position where this record started
            write_pos = lseek(fd, 0, SEEK_CUR) - sizeof(Report);
            break;
        }
    }

    if (write_pos == -1) {
        printf("Report %d not found.\n", target_id);
        close(fd);
        return;
    }

    // 2. Shift subsequent records backward
    Report temp;
    off_t read_pos = write_pos + sizeof(Report); // Start reading from the record AFTER the deleted one

    while (1) {
        // Jump to read position
        lseek(fd, read_pos, SEEK_SET);
        if (read(fd, &temp, sizeof(Report)) != sizeof(Report)) {
            break; // End of file reached
        }

        // Save where we need to read from next time
        read_pos = lseek(fd, 0, SEEK_CUR);

        // Jump to write position and overwrite
        lseek(fd, write_pos, SEEK_SET);
        write(fd, &temp, sizeof(Report));

        // Update write position for the next loop
        write_pos += sizeof(Report);
    }

    // 3. Truncate the file to chop off the duplicate last record
    if (ftruncate(fd, write_pos) == -1) {
        perror("Failed to truncate file");
    } else {
        printf("Report %d successfully removed.\n", target_id);
    }

    close(fd);
}

void update_threshold(const char *district_id, int new_threshold) {
    char path[512];
    snprintf(path, sizeof(path), "%s/district.cfg", district_id);

    struct stat st;
    if (stat(path, &st) == -1) {
        perror("Could not stat district.cfg");
        return;
    }

    // Extract only the permission bits from st_mode
    mode_t perms = st.st_mode & 0777;

    // The project explicitly requires strictly checking if it is exactly 0640
    if (perms != 0640) {
        printf("DIAGNOSTIC ERROR: district.cfg permissions have been altered (current: %o, expected: 640). Aborting update.\n", perms);
        return;
    }

    // O_TRUNC wipes the existing contents before we write the new threshold
    int fd = open(path, O_WRONLY | O_TRUNC);
    if (fd == -1) {
        perror("Failed to open district.cfg");
        return;
    }

    char buf[32];
    snprintf(buf, sizeof(buf), "%d\n", new_threshold);
    write(fd, buf, strlen(buf));

    printf("Threshold updated to %d.\n", new_threshold);
    close(fd);
}
