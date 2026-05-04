# AI Assistant Usage Log (Phases 1 & 2)

## Tool Used
I used Anthropic's Claude 4.6 Opus / OpenAI GPT-4 to assist with tricky C string parsing in Phase 1 and signal handling boilerplate in Phase 2.

---

## Phase 1 Usage

### `parse_condition` Generation
**My Prompt:**
> I am writing a C program to manage city infrastructure reports. I need a function with the signature:
> `int parse_condition(const char *input, char *field, char *op, char *value);`
> It needs to split an input string of the form "field:operator:value" into its three parts and store them in the pre-allocated buffers `field`, `op`, and `value`. It should return 1 on success and 0 on failure.

**What was generated & What I changed:**
The provided code was technically correct and robust, using `strchr` safely. I incorporated it into my codebase without major changes. I added additional buffer size limits explicitly ensuring destination buffers didn't overflow their 32 and 255 character bounds before `strncpy`.

### `match_condition` Generation
**My Prompt:**
> I need a function generated with the signature:
> `int match_condition(Report *r, const char *field, const char *op, const char *value);`
> It should take the extracted fields and value, convert the value string into the appropriate C type, and execute the correct comparison based on the operator string ("==", "!=", "<", "<=", ">", ">="). It returns 1 if matched, 0 if not. Support the fields: "severity", "category", "inspector", "timestamp".

**What was generated & What I changed:**
The AI's output lacked string comparisons for `<` or `>`, which is fine because category and inspector only realistically support `==` and `!=`. I refined the string `strcmp(op, "==") == 0` blocks to be clearer by organizing it by field first, then operator.

---

## Phase 2 Usage

### `sigaction` Handler Generation
**My Prompt:**
> I need a boilerplate C program that demonstrates how to safely use `sigaction` instead of `signal()` to handle `SIGINT` and `SIGUSR1`. When `SIGINT` is received, it should break a loop and exit. When `SIGUSR1` is received, it should print a message.

**What was generated:**
```c
#include <stdio.h>
#include <signal.h>
#include <unistd.h>

volatile sig_atomic_t keep_running = 1;

void sigint_handler(int sig) {
    keep_running = 0;
}

void sigusr1_handler(int sig) {
    const char *msg = "SIGUSR1 received\n";
    write(STDOUT_FILENO, msg, 17);
}

int main() {
    struct sigaction sa_int;
    sa_int.sa_handler = sigint_handler;
    sigemptyset(&sa_int.sa_mask);
    sa_int.sa_flags = 0;
    sigaction(SIGINT, &sa_int, NULL);
    // ... same for SIGUSR1
}
```

**What I changed and why:**
The AI correctly noted that `printf` is NOT async-signal-safe, and used `write(STDOUT_FILENO, ...)` inside the signal handler instead. This was a crucial reminder. I adapted this generated code into `monitor_reports.c`, altering the messages to fit the "New report added!" requirement.

## What I Learned
- AI is excellent at writing boilerplate C code like `sigaction` setup, which is otherwise tedious to type out manually.
- The AI's warning about async-signal-safe functions (like `write` vs `printf`) in signal handlers was a great reinforcement of C system programming best practices.
