//
// Created by adna on 4/19/26.
//

#ifndef CITYINFRASTRUCTURE_MODELS_H // din clion
#define CITYINFRASTRUCTURE_MODELS_H // din clion

#ifndef MODELS_H
#define MODELS_H

#include <time.h>

/* Define fixed lengths for strings to avoid binary corruption */
#define MAX_NAME_LEN 32
#define MAX_CATEGORY_LEN 16
#define MAX_DESC_LEN 256

/* The core data structure stored in reports.dat */
typedef struct {
    int report_id;
    char inspector_name[MAX_NAME_LEN];
    double latitude;
    double longitude;
    char category[MAX_CATEGORY_LEN];
    int severity;
    time_t timestamp;
    char description[MAX_DESC_LEN];
} Report;

#endif /* MODELS_H */

#endif //CITYINFRASTRUCTURE_MODELS_H // din clion
