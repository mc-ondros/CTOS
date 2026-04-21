# AI Assistant Usage Log

## Tool Used
I used Anthropic's Claude 4.6 Opus / OpenAI GPT-4 to generate the required condition parsing and matching functions.

## `parse_condition` Generation

**My Prompt:**
> I am writing a C program to manage city infrastructure reports. I need a function with the signature:
> `int parse_condition(const char *input, char *field, char *op, char *value);`
> It needs to split an input string of the form "field:operator:value" into its three parts and store them in the pre-allocated buffers `field`, `op`, and `value`. It should return 1 on success and 0 on failure.

**What was generated:**
```c
int parse_condition(const char *input, char *field, char *op, char *value) {
    if (input == NULL || field == NULL || op == NULL || value == NULL) {
        return 0;
    }
    
    const char *first_colon = strchr(input, ':');
    if (!first_colon) return 0;
    
    const char *second_colon = strchr(first_colon + 1, ':');
    if (!second_colon) return 0;
    
    size_t field_len = first_colon - input;
    size_t op_len = second_colon - (first_colon + 1);
    
    strncpy(field, input, field_len);
    field[field_len] = '\0';
    
    strncpy(op, first_colon + 1, op_len);
    op[op_len] = '\0';
    
    strcpy(value, second_colon + 1);
    
    return 1;
}
```

**What I changed and why:**
The provided code was technically correct and robust, using `strchr` safely. I incorporated it into my codebase without major changes. However, I noticed I had to be careful with the lengths of my destination buffers (to prevent buffer overflows). I added additional buffer size limits implicitly by ensuring my destination buffers were larger than the maximum expected input.

## `match_condition` Generation

**My Prompt:**
> My C program has a structure for reports:
> ```c
> typedef struct {
>     int id;
>     char inspector[32];
>     double latitude;
>     double longitude;
>     char category[32];
>     int severity;
>     time_t timestamp;
>     char description[256];
> } Report;
> ```
> I need a function generated with the signature:
> `int match_condition(Report *r, const char *field, const char *op, const char *value);`
> It should take the extracted fields and value, convert the value string into the appropriate C type (like int for severity and timestamp, string for category and inspector), and execute the correct comparison based on the operator string ("==", "!=", "<", "<=", ">", ">="). It returns 1 if matched, 0 if not. Support the fields: "severity", "category", "inspector", "timestamp".

**What was generated:**
The AI generated a large function using `strcmp` on the `field`, then `strcmp` on `op`, followed by value conversions (e.g., `atoi(value)`) and the actual logical comparisons. 

**What I changed and why:**
The AI's generated output lacked string comparisons for `<` or `>`, which is fine because category and inspector only realistically support `==` and `!=`. However, I had to ensure that the code explicitly handled the conversion of strings for timestamp and severity before executing integer comparison. I refined the string `strcmp(op, "==") == 0` blocks to be clearer and less prone to nested `if`-spaghetti by organizing it by field first, then operator.

## What I Learned
- AI generated string parsing functions correctly, but dealing with memory lifetimes and buffer bounds still requires a human engineer to ensure safety in C.
- Passing structured and explicit prompts (giving the exact struct and signature) yielded immediately useful code that saved a lot of boilerplate typing for `strcmp` and type conversions.
- Always check the data types that the AI uses to convert values from strings (`atoi` for ints, `atol` / `atoll` for timestamps) to ensure correct sizes.
