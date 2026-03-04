#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <windows.h>

static int cmp_entries(const void *a, const void *b) {
    const char *string_a = *(const char **)a;
    const char *string_b = *(const char **)b;

    return strcmp(string_a, string_b);
}

static int get_terminal_width() {
    CONSOLE_SCREEN_BUFFER_INFO csbi;
    if (GetConsoleScreenBufferInfo(GetStdHandle(STD_OUTPUT_HANDLE), &csbi)) {
        return csbi.srWindow.Right - csbi.srWindow.Left + 1;
    }

    return 120; // Default terminal width
}

static void print_columns(char **entries, const int count) {
    if (count == 0) {
        return;
    }

    const int term_width = get_terminal_width();

    int max_entry_len = 0;
    for (int i = 0; i < count; i++) {
        int entry_len = (int)strlen(entries[i]);
        if (entry_len > max_entry_len) {
            max_entry_len = entry_len;
        }
    }

    int col_width = max_entry_len + 2;
    int num_cols = max(term_width / col_width, 1);
    int num_rows = (count + num_cols - 1) / num_cols;

    for (int r = 0; r < num_rows; r++) {
        for (int c = 0; c < num_cols; c++) {
            int entry_index = c * num_rows + r;
            if (entry_index >= count) {
                break;
            }

            if ((c < num_cols - 1) && ((c + 1) * num_rows + r < count)) {
                printf("%-*s", col_width, entries[entry_index]);
            } else {
                printf("%s", entries[entry_index]);
            }
        }

        printf("\n");
    }
}

int main(const int argc, char *argv[]) {
    const char *target_directory = NULL;
    int flag_all = 0;
    int flag_reverse = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            printf("Usage: ls [OPTION]... [FILE]...\n");
            printf("\nOptions:\n");
            printf("  -a, --all                  do not ignore entries starting with .\n");
            printf("  -r, --reverse              reverse order while sorting\n");
            printf("      --help     display this help and exit\n");
            printf("      --version  output version information and exit\n");

            return 0;
        }

        if (strcmp(argv[i], "--version") == 0) {
            printf("ls (windows-ls) 0.1\n");
            printf("License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.");
            printf("This is free software: you are free to change and redistribute it.");
            printf("There is NO WARRANTY, to the extent permitted by law.\n");
            return 0;
        }

        if (strcmp(argv[i], "--all") == 0) {
            flag_all = 1;
        } else if (strcmp(argv[i], "--reverse") == 0) {
            flag_reverse = 1;
        } else if (argv[i][0] == '-') {
            for (int j = 1; argv[i][j] != '\0'; j++) {
                switch (argv[i][j]) {
                    case 'a':
                        flag_all = 1;
                        break;
                    case 'r':
                        flag_reverse = 1;
                        break;
                    default:
                        fprintf(stderr, "windows-ls: invalid option -- '%c'\n", argv[i][j]);
                        fprintf(stderr, "Try 'ls --help' for more information.\n");
                        return 1;
                }
            }
        } else {
            target_directory = argv[i];
        }
    }

    char cwd[MAX_PATH];
    if (target_directory != NULL) {
        strncpy(cwd, target_directory, sizeof(cwd) - 1);
        cwd[sizeof(cwd) - 1] = '\0';
    } else {
        if (getcwd(cwd, sizeof(cwd)) == NULL) {
            fprintf(stderr, "windows-ls: cannot get current directory");
            return 1;
        }
    }

    DIR *directory = opendir(cwd);
    if (directory == NULL) {
        fprintf(stderr, "windows-ls: cannot access '%s': No such file or directory\n", cwd);
        return 1;
    }

    int count = 0;
    int capacity = 16;
    char **entries = malloc(capacity * sizeof(char *));
    if (!entries) {
        closedir(directory);
        return 2;
    }

    struct dirent *directory_entry;
    while ((directory_entry = readdir(directory)) != NULL) {
        if (!flag_all && directory_entry->d_name[0] == '.') {
            continue;
        }

        if (count >= capacity) {
            capacity *= 2;
            char **new_entries = realloc(entries, capacity * sizeof(char *));
            if (!new_entries) {
                for (int i = 0; i < count; i++) {
                    free(entries[i]);
                    entries[i] = NULL;
                }
                free(entries);
                entries = NULL;

                closedir(directory);
                return 2;
            }

            entries = new_entries;
        }

        entries[count] = malloc(sizeof(char) + strlen(directory_entry->d_name) + 1);
        if (!entries[count]) {
            for (int i = 0; i < count; i++) {
                free(entries[i]);
            }
            free(entries);
            closedir(directory);
            return 2;
        }

        strcpy(entries[count], directory_entry->d_name);
        count++;
    }
    closedir(directory);

    qsort(entries, count, sizeof(char *), cmp_entries);
    if (flag_reverse) {
        for (int i = 0; i < count / 2; i++) {
            char *tmp = entries[i];
            entries[i] = entries[count - i - 1];
            entries[count - i - 1] = tmp;
        }
    }

    print_columns(entries, count);

    for (int i = 0; i < count; i++) {
        free(entries[i]);
        entries[i] = NULL;
    }
    free(entries);
    entries = NULL;

    return 0;
}