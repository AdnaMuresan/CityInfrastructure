# AI Usage Documentation - Phase 1

**Tool Used:** Gemini

## 1. Generating `parse_condition`
**Prompt:** "I am building a C program. I have a string formatted as `field:operator:value` (e.g., `severity:>=:2`). Generate a C function `int parse_condition(const char *input, char *field, char *op, char *value);` that splits this string into its three parts."

**What was generated:** The AI generated a function using `strchr` to locate the colons and pointer arithmetic to calculate the lengths of the substrings, using `strncpy` and `strcpy` to populate the output buffers.

**What I learned/changed:** I learned that using standard string parsing like `sscanf` can be risky if the `value` contains spaces, so `strchr` is much safer. I made sure to verify that the buffers passed into the function (e.g., `char field[32]`) are large enough to handle the lengths calculated by pointer arithmetic to prevent buffer overflows.

## 2. Generating `match_condition`
**Prompt:** "My C struct has the following fields: `int severity`, `char category[16]`, `char inspector_name[32]`, and `time_t timestamp`. Generate a function `int match_condition(Report *r, const char *field, const char *op, const char *value);` that returns 1 if the struct matches the condition and 0 otherwise. Handle ==, !=, <, <=, >, >=."

**What was generated:** The AI generated a massive `if/else if` chain comparing the `field` string. For integers/time, it converted the `value` string using `atoi()` and `atol()`. For strings, it used `strcmp()`.

**What I learned/changed:** The most valuable lesson was handling data type conversions. The terminal passes arguments as text strings, meaning the `value` is always a `char*`. You cannot compare an integer `severity` to a string `"2"`. The AI correctly implemented `atoi(value)` before doing the mathematical comparison `<` or `>`. I manually reviewed the string logic to ensure it only applied `==` and `!=` to `category` and `inspector`, since applying `<` to a string conceptually doesn't fit the scope of this project.