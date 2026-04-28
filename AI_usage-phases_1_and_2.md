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

## Phase 2: Processes and Signals

**Tools Used:** Gemini

### 1. Generating the Background Monitor and `sigaction`
**Prompt:** "I need to write a background C program called `monitor_reports` that loops indefinitely until it receives a SIGINT. It also needs to listen for SIGUSR1. I am forbidden from using the basic `signal()` function. Show me how to set this up using `sigaction()` and how to safely print a message when a signal is received."

**What was generated:** The AI generated the boilerplate for `sigaction`, including setting `sa.sa_flags = 0` and `sigemptyset()`. Crucially, inside the signal handlers, the AI used the `write(STDOUT_FILENO, ...)` system call instead of `printf()`.

**What I learned/changed:** I learned that `printf` is not "async-signal-safe." If a signal interrupts the program while it is already in the middle of a `printf` call, calling `printf` again inside the signal handler can cause a deadlock or crash. Using `write()` directly to standard output is the POSIX-compliant, safe way to print from inside a signal handler. I also learned to use a `volatile sig_atomic_t` flag to safely break the `pause()` loop when SIGINT is caught.

### 2. Deleting the District (`fork` and `exec`)
**Prompt:** "I need to add a `--remove_district` command that deletes a folder. The project requires me to use `fork()` to create a child process, and then use the `exec*()` family to run the terminal command `rm -rf <district_directory>`. How do I structure this safely?"

**What was generated:** The AI provided a standard `fork()` template (`if pid == 0` for child, `else` for parent). In the child, it used `execlp("rm", "rm", "-rf", district_id, NULL)`. In the parent, it used `wait(NULL)` to pause until the child finished before unlinking the symlink.

**What I learned/changed:** I learned how `exec` actually works: it completely overwrites the child process's memory space with the new program (`rm`). That is why you don't need a `return` or `exit` statement after a successful `exec`—the original C code no longer exists in that process! I added an `exit(1)` immediately after the `execlp` call just in case the execution fails, ensuring a broken child process doesn't accidentally continue running the rest of the parent's code.
