//
// Created by adna on 4/19/26.
//

#ifndef CITYINFRASTRUCTURE_FS_UTILS_H // clion
#define CITYINFRASTRUCTURE_FS_UTILS_H // clion

#ifndef FS_UTILS_H
#define FS_UTILS_H

#include <sys/types.h>
#include <sys/stat.h>

/* Creates a file, writes optional initial data, and enforces strict permissions */
void create_and_chmod(const char *filepath, mode_t permissions, const char *initial_data);

/* Initializes the district directory and all required files inside it */
void init_district(const char *district_id);

/* Converts a st_mode bitmask into a 9-character string (e.g., rw-rw-r--) */
void mode_to_string(mode_t mode, char *str);

/* Creates the active_reports-<district_id> symlink */
void setup_symlink(const char *district_id);

/* Uses lstat() to check if the symlink is dangling */
void check_symlink_health(const char *district_id);

#endif /* FS_UTILS_H */

#endif //CITYINFRASTRUCTURE_FS_UTILS_H //clion
