#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>

#define ROLE_MANAGER 1
#define ROLE_INSPECTOR 2
#define ROLE_UNKNOWN 0

typedef struct {
    int id;
    char inspector[32];
    double latitude;
    double longitude;
    char category[32];
    int severity;
    time_t timestamp;
    char description[256];
} Report;

int current_role = ROLE_UNKNOWN;
char current_user[64] = "";
const char *role_str = "unknown";

void print_permissions(mode_t mode, char *str) {
    strcpy(str, "---------");
    if (mode & S_IRUSR) str[0] = 'r';
    if (mode & S_IWUSR) str[1] = 'w';
    if (mode & S_IXUSR) str[2] = 'x';
    if (mode & S_IRGRP) str[3] = 'r';
    if (mode & S_IWGRP) str[4] = 'w';
    if (mode & S_IXGRP) str[5] = 'x';
    if (mode & S_IROTH) str[6] = 'r';
    if (mode & S_IWOTH) str[7] = 'w';
    if (mode & S_IXOTH) str[8] = 'x';
}

int check_access(const char *path, int req_r, int req_w, int req_x) {
    struct stat st;
    if (stat(path, &st) < 0) {
        if (errno == ENOENT) return 1; // Allows creating files if missing, checked elsewhere
        perror("stat");
        return 0;
    }
    mode_t mode = st.st_mode;
    if (current_role == ROLE_MANAGER) {
        if (req_r && !(mode & S_IRUSR)) return 0;
        if (req_w && !(mode & S_IWUSR)) return 0;
        if (req_x && !(mode & S_IXUSR)) return 0;
    } else if (current_role == ROLE_INSPECTOR) {
        if (req_r && !(mode & S_IRGRP)) return 0;
        if (req_w && !(mode & S_IWGRP)) return 0;
        if (req_x && !(mode & S_IXGRP)) return 0;
    } else {
        return 0;
    }
    return 1;
}

void require_access(const char *path, int r, int w, int x, const char *op_name) {
    if (!check_access(path, r, w, x)) {
        fprintf(stderr, "Permission denied for role %s to %s file %s.\n", role_str, op_name, path);
        exit(1);
    }
}

void try_log_action(const char *district, const char *action) {
    char path[256];
    snprintf(path, sizeof(path), "%s/logged_district", district);
    
    // Check permission to write log
    if (!check_access(path, 0, 1, 0)) {
        printf("[Log] Inspector role restricted from writing to operation log.\n");
        return;
    }
    
    int fd = open(path, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (fd < 0) {
        perror("open log");
        return;
    }
    // ensure permissions are 644
    chmod(path, 0644);
    
    time_t now = time(NULL);
    char buf[512];
    int len = snprintf(buf, sizeof(buf), "[%ld] Role: %s, User: %s, Action: %s\n", 
                       (long)now, role_str, current_user, action);
    write(fd, buf, len);
    close(fd);
}

void setup_symlink(const char *district) {
    char link_name[256];
    char target[256];
    snprintf(link_name, sizeof(link_name), "active_reports-%s", district);
    snprintf(target, sizeof(target), "%s/reports.dat", district);

    struct stat lst;
    if (lstat(link_name, &lst) == 0) {
        if (S_ISLNK(lst.st_mode)) {
            struct stat st;
            if (stat(link_name, &st) != 0) {
                printf("Warning: dangling symlink %s detected. Cleaning up.\n", link_name);
                unlink(link_name);
            } else {
                return; // Symlink is fine
            }
        }
    }
    symlink(target, link_name);
}

void setup_district(const char *district) {
    struct stat st;
    if (stat(district, &st) < 0) {
        if (mkdir(district, 0750) < 0) {
            perror("mkdir");
            exit(1);
        }
        chmod(district, 0750);
    }

    require_access(district, 1, 0, 1, "access directory");

    char path[256];
    
    // Check/create district.cfg
    snprintf(path, sizeof(path), "%s/district.cfg", district);
    if (stat(path, &st) < 0) {
        // Assume manager can create it, inspectors can't
        if (current_role == ROLE_INSPECTOR) {
            fprintf(stderr, "Inspector cannot initialize missing configuration.\n");
            exit(1);
        }
        int fd = open(path, O_WRONLY | O_CREAT, 0640);
        if (fd >= 0) {
            write(fd, "1\n", 2);
            close(fd);
            chmod(path, 0640);
        }
    }

    // Check/create reports.dat
    snprintf(path, sizeof(path), "%s/reports.dat", district);
    if (stat(path, &st) < 0) {
        int fd = open(path, O_WRONLY | O_CREAT, 0664);
        if (fd >= 0) {
            close(fd);
            chmod(path, 0664);
        }
    }

    // Check/create logged_district
    snprintf(path, sizeof(path), "%s/logged_district", district);
    if (stat(path, &st) < 0) {
        if (current_role == ROLE_MANAGER) {
            int fd = open(path, O_WRONLY | O_CREAT, 0644);
            if (fd >= 0) {
                close(fd);
                chmod(path, 0644);
            }
        }
    }

    setup_symlink(district);
}

// ============== AI GENERATED FUNCTIONS ==============
int parse_condition(const char *input, char *field, char *op, char *value) {
    if (input == NULL || field == NULL || op == NULL || value == NULL) return 0;
    const char *first_colon = strchr(input, ':');
    if (!first_colon) return 0;
    const char *second_colon = strchr(first_colon + 1, ':');
    if (!second_colon) return 0;
    size_t field_len = first_colon - input;
    size_t op_len = second_colon - (first_colon + 1);
    if (field_len >= 32) field_len = 31;
    strncpy(field, input, field_len);
    field[field_len] = '\0';
    if (op_len >= 4) op_len = 3;
    strncpy(op, first_colon + 1, op_len);
    op[op_len] = '\0';
    strncpy(value, second_colon + 1, 255);
    value[255] = '\0';
    return 1;
}

int match_condition(Report *r, const char *field, const char *op, const char *value) {
    if (strcmp(field, "severity") == 0) {
        int v = atoi(value);
        if (strcmp(op, "==") == 0) return r->severity == v;
        if (strcmp(op, "!=") == 0) return r->severity != v;
        if (strcmp(op, "<") == 0) return r->severity < v;
        if (strcmp(op, "<=") == 0) return r->severity <= v;
        if (strcmp(op, ">") == 0) return r->severity > v;
        if (strcmp(op, ">=") == 0) return r->severity >= v;
    } else if (strcmp(field, "timestamp") == 0) {
        long v = atol(value);
        if (strcmp(op, "==") == 0) return r->timestamp == v;
        if (strcmp(op, "!=") == 0) return r->timestamp != v;
        if (strcmp(op, "<") == 0) return r->timestamp < v;
        if (strcmp(op, "<=") == 0) return r->timestamp <= v;
        if (strcmp(op, ">") == 0) return r->timestamp > v;
        if (strcmp(op, ">=") == 0) return r->timestamp >= v;
    } else if (strcmp(field, "category") == 0) {
        int cmp = strcmp(r->category, value);
        if (strcmp(op, "==") == 0) return cmp == 0;
        if (strcmp(op, "!=") == 0) return cmp != 0;
    } else if (strcmp(field, "inspector") == 0) {
        int cmp = strcmp(r->inspector, value);
        if (strcmp(op, "==") == 0) return cmp == 0;
        if (strcmp(op, "!=") == 0) return cmp != 0;
    }
    return 0;
}
// ====================================================

void do_add(const char *district) {
    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district);
    require_access(path, 0, 1, 0, "write");

    int fd = open(path, O_RDWR);
    if (fd < 0) {
        perror("open reports.dat");
        exit(1);
    }

    Report r;
    memset(&r, 0, sizeof(r));
    // calculate ID: read last report if exists to get count
    off_t size = lseek(fd, 0, SEEK_END);
    int num_reports = size / sizeof(Report);
    r.id = (num_reports > 0) ? (num_reports + 1) : 1;
    strncpy(r.inspector, current_user, sizeof(r.inspector)-1);
    r.timestamp = time(NULL);
    
    // Fill with dummy data for this example implementation
    printf("Enter category (e.g., road, lighting): ");
    scanf("%31s", r.category);
    printf("Enter severity (1-3): ");
    scanf("%d", &r.severity);
    printf("Enter description: ");
    while(getchar() != '\n'); // clear input buffer
    fgets(r.description, sizeof(r.description), stdin);
    r.description[strcspn(r.description, "\n")] = 0; // Remove newline

    r.latitude = 0.0;
    r.longitude = 0.0;

    write(fd, &r, sizeof(Report));
    close(fd);

    printf("Report added with ID %d.\n", r.id);
    char action[128];
    snprintf(action, sizeof(action), "Add report %d", r.id);
    try_log_action(district, action);
}

void do_list(const char *district) {
    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district);
    require_access(path, 1, 0, 0, "read");

    struct stat st;
    if (stat(path, &st) < 0) {
        perror("stat reports.dat");
        exit(1);
    }
    
    char perm_str[10];
    print_permissions(st.st_mode, perm_str);
    
    char time_str[64];
    struct tm *tm_info = localtime(&st.st_mtime);
    strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", tm_info);

    printf("File info: %s | Size: %ld bytes | Last Mod: %s\n", perm_str, (long)st.st_size, time_str);
    printf("--- Reports ---\n");

    int fd = open(path, O_RDONLY);
    if (fd < 0) return;

    Report r;
    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        printf("ID: %d | Inspector: %s | Category: %s | Severity: %d\n", r.id, r.inspector, r.category, r.severity);
    }
    close(fd);

    try_log_action(district, "List reports");
}

void do_view(const char *district, int report_id) {
    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district);
    require_access(path, 1, 0, 0, "read");

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("open reports.dat");
        exit(1);
    }

    Report r;
    int found = 0;
    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        if (r.id == report_id) {
            printf("ID: %d\n", r.id);
            printf("Inspector: %s\n", r.inspector);
            printf("Timestamp: %ld\n", (long)r.timestamp);
            printf("Category: %s\n", r.category);
            printf("Severity: %d\n", r.severity);
            printf("Description: %s\n", r.description);
            found = 1;
            break;
        }
    }
    close(fd);
    if (!found) {
        printf("Report %d not found.\n", report_id);
    }

    char action[128];
    snprintf(action, sizeof(action), "View report %d", report_id);
    try_log_action(district, action);
}

void do_remove(const char *district, int report_id) {
    if (current_role != ROLE_MANAGER) {
        fprintf(stderr, "Only manager can remove reports.\n");
        exit(1);
    }

    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district);
    require_access(path, 1, 1, 0, "read/write");

    int fd = open(path, O_RDWR);
    if (fd < 0) {
        perror("open reports.dat");
        exit(1);
    }

    struct stat st_before;
    fstat(fd, &st_before);

    off_t pos = 0;
    Report r;
    int target_idx = -1;
    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        if (r.id == report_id) {
            target_idx = pos / sizeof(Report);
            break;
        }
        pos += sizeof(Report);
    }

    if (target_idx == -1) {
        printf("Report %d not found.\n", report_id);
        close(fd);
        return;
    }

    // Shift data
    off_t write_pos = target_idx * sizeof(Report);
    off_t read_pos = (target_idx + 1) * sizeof(Report);
    
    while (1) {
        lseek(fd, read_pos, SEEK_SET);
        int bytes = read(fd, &r, sizeof(Report));
        if (bytes <= 0) break;
        lseek(fd, write_pos, SEEK_SET);
        write(fd, &r, sizeof(Report));
        write_pos += sizeof(Report);
        read_pos += sizeof(Report);
    }

    ftruncate(fd, write_pos);
    
    struct stat st_after;
    fstat(fd, &st_after);
    printf("Report %d removed. File size changed from %ld to %ld.\n", report_id, (long)st_before.st_size, (long)st_after.st_size);

    close(fd);

    char action[128];
    snprintf(action, sizeof(action), "Remove report %d", report_id);
    try_log_action(district, action);
}

void do_update_threshold(const char *district, int value) {
    if (current_role != ROLE_MANAGER) {
        fprintf(stderr, "Only manager can update threshold.\n");
        exit(1);
    }

    char path[256];
    snprintf(path, sizeof(path), "%s/district.cfg", district);
    
    // Explicit 640 check
    struct stat st;
    if (stat(path, &st) == 0) {
        if ((st.st_mode & 0777) != 0640) {
            fprintf(stderr, "Error: %s permissions have been altered from 0640. Refusing operation.\n", path);
            exit(1);
        }
    }
    
    require_access(path, 1, 1, 0, "read/write");

    int fd = open(path, O_WRONLY | O_TRUNC);
    if (fd < 0) {
        perror("open district.cfg");
        exit(1);
    }

    char buf[32];
    int len = snprintf(buf, sizeof(buf), "%d\n", value);
    write(fd, buf, len);
    close(fd);

    printf("Threshold updated to %d.\n", value);

    char action[128];
    snprintf(action, sizeof(action), "Update threshold to %d", value);
    try_log_action(district, action);
}

void do_filter(const char *district, int num_conditions, char **conditions) {
    char path[256];
    snprintf(path, sizeof(path), "%s/reports.dat", district);
    require_access(path, 1, 0, 0, "read");

    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("open reports.dat");
        exit(1);
    }

    Report r;
    while (read(fd, &r, sizeof(Report)) == sizeof(Report)) {
        int all_match = 1;
        for (int i = 0; i < num_conditions; i++) {
            char field[32], op[8], value[256];
            if (!parse_condition(conditions[i], field, op, value)) {
                fprintf(stderr, "Failed to parse condition: %s\n", conditions[i]);
                all_match = 0;
                break;
            }
            if (!match_condition(&r, field, op, value)) {
                all_match = 0;
                break;
            }
        }
        if (all_match) {
             printf("ID: %d | Inspector: %s | Category: %s | Severity: %d | Time: %ld\n", 
                    r.id, r.inspector, r.category, r.severity, (long)r.timestamp);
        }
    }
    close(fd);

    try_log_action(district, "Filter reports");
}

int main(int argc, char *argv[]) {
    if (argc < 4) {
        fprintf(stderr, "Usage: %s --role <role> --user <user> --<cmd> <district> [args...]\n", argv[0]);
        return 1;
    }

    int cmd_index = -1;
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--role") == 0 && i + 1 < argc) {
            if (strcmp(argv[i+1], "manager") == 0) current_role = ROLE_MANAGER;
            else if (strcmp(argv[i+1], "inspector") == 0) current_role = ROLE_INSPECTOR;
            role_str = argv[i+1];
            i++;
        } else if (strcmp(argv[i], "--user") == 0 && i + 1 < argc) {
            strncpy(current_user, argv[i+1], sizeof(current_user)-1);
            i++;
        } else if (strncmp(argv[i], "--", 2) == 0 && cmd_index == -1) {
            cmd_index = i;
        }
    }

    if (current_role == ROLE_UNKNOWN) {
        fprintf(stderr, "Unknown or missing role.\n");
        return 1;
    }

    if (cmd_index == -1 || cmd_index + 1 >= argc) {
        fprintf(stderr, "Missing command or district.\n");
        return 1;
    }

    char *cmd = argv[cmd_index] + 2; // skip "--"
    char *district = argv[cmd_index + 1];

    setup_district(district);

    if (strcmp(cmd, "add") == 0) {
        do_add(district);
    } else if (strcmp(cmd, "list") == 0) {
        do_list(district);
    } else if (strcmp(cmd, "view") == 0) {
        if (cmd_index + 2 >= argc) { fprintf(stderr, "Missing report_id.\n"); return 1; }
        do_view(district, atoi(argv[cmd_index + 2]));
    } else if (strcmp(cmd, "remove_report") == 0) {
        if (cmd_index + 2 >= argc) { fprintf(stderr, "Missing report_id.\n"); return 1; }
        do_remove(district, atoi(argv[cmd_index + 2]));
    } else if (strcmp(cmd, "update_threshold") == 0) {
        if (cmd_index + 2 >= argc) { fprintf(stderr, "Missing value.\n"); return 1; }
        do_update_threshold(district, atoi(argv[cmd_index + 2]));
    } else if (strcmp(cmd, "filter") == 0) {
        do_filter(district, argc - (cmd_index + 2), &argv[cmd_index + 2]);
    } else {
        fprintf(stderr, "Unknown command: %s\n", cmd);
        return 1;
    }

    return 0;
}
