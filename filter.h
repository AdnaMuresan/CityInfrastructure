//
// Created by adna on 4/19/26.
//

#ifndef CITYINFRASTRUCTURE_FILTER_H
#define CITYINFRASTRUCTURE_FILTER_H

#ifndef FILTER_H
#define FILTER_H

#include "models.h"

/* AI-Generated: Parses "field:operator:value" into separate strings */
int parse_condition(const char *input, char *field, char *op, char *value);

/* AI-Generated: Returns 1 if the record matches the condition, 0 otherwise */
int match_condition(Report *r, const char *field, const char *op, const char *value);

/* Human-Written: The main logic that reads the file and applies the filters */
void filter_reports(const char *district_id, char *conditions[], int num_conditions);

#endif /* FILTER_H */

#endif //CITYINFRASTRUCTURE_FILTER_H
