#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <windows.h>

typedef struct {
    bool all;
    bool almost_all;
    bool reverse;
    bool nosort;
    int term_width;
    const char *target_directory;
} LsOptions;

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

static void print_columns(char **entries, const int count, const int col_width, const int num_cols) {
    if (count == 0) {
        return;
    }

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

static void print_help() {
    printf("Usage: ls [OPTION]... [DIRECTORY]...\n");
    printf("\nOptions:\n");
    printf("  -a, --all                  do not ignore entries starting with .\n");
    printf("  -A, --almost-all           do not list implied . and ..\n");
    printf("  -f                         do not sort, enable -aU\n");
    printf("  -r, --reverse              reverse order while sorting\n");
    printf("  -U                         do not sort; list entries in directory order\n");
    printf("  -w, --width=COLS           set output width to COLS.  0 means no limit\n");
    printf("  -1                         list one file per line\n");
    printf("      --help     display this help and exit\n");
    printf("      --version  output version information and exit\n");
}

static void print_version() {
    printf("ls (windows-ls) 1.1\n");
    printf("License GPLv3+: GNU GPL version 3 or later <https://gnu.org/licenses/gpl.html>.\n");
    printf("This is free software: you are free to change and redistribute it.\n");
    printf("There is NO WARRANTY, to the extent permitted by law.\n");
}

static int parse_arguments(int argc, char *argv[], LsOptions *ls_options) {
    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "--help") == 0) {
            print_help();
            return 1;
        }

        if (strcmp(argv[i], "--version") == 0) {
            print_version();
            return 1;
        }

        if (strcmp(argv[i], "--all") == 0) {
            ls_options->all = true;
        } else if (strcmp(argv[i], "--almost-all") == 0) {
            ls_options->almost_all = true;
        } else if (strcmp(argv[i], "--reverse") == 0) {
            ls_options->reverse = true;
        } else if (strncmp(argv[i], "--width=", 8) == 0) {
            ls_options->term_width = strtol(argv[i] + 8, NULL, 10);
        } else if (strcmp(argv[i], "--width") == 0) {
            if (i + 1 < argc) {
                ls_options->term_width = strtol(argv[i + 1], NULL, 10);
                i++;
            } else {
                fprintf(stderr, "windows-ls: option '--width' requires an argument\n");
                return -1;
            }
        } else if (argv[i][0] == '-') {
            bool skip_rest = false;
            for (int j = 1; argv[i][j] != '\0' && !skip_rest; j++) {
                switch (argv[i][j]) {
                    case 'a':
                        ls_options->all = true;
                        break;
                    case 'A':
                        ls_options->almost_all = true;
                        break;
                    case 'f':
                        ls_options->all = true;
                        ls_options->nosort = true;
                        break;
                    case 'r':
                        ls_options->reverse = true;
                        break;
                    case 'U':
                        ls_options->nosort = true;
                        break;
                    case 'w':
                        if (argv[i][j + 1] != '\0') {
                            char *value = &argv[i][j + 1];
                            if (value[0] == '=') {
                                fprintf(stderr, "windows-ls: invalid line width: '%s'\n", value);
                                return -1;
                            }
                            ls_options->term_width = strtol(value, NULL, 10);
                            skip_rest = true;
                        } else {
                            if (i + 1 < argc) {
                                ls_options->term_width = strtol(argv[i + 1], NULL, 10);
                                i++;
                                skip_rest = true;
                            } else {
                                fprintf(stderr, "windows-ls: option requires an argument -- 'w'\n");
                                return -1;
                            }
                        }
                        break;
                    case '1':
                        ls_options->term_width = 1;
                        break;
                    default:
                        fprintf(stderr, "windows-ls: invalid option -- '%c'\n", argv[i][j]);
                        fprintf(stderr, "Try 'ls --help' for more information.\n");
                        return -1;
                }
            }
        } else {
            ls_options->target_directory = argv[i];
        }
    }
    return 0;
}

int main(const int argc, char *argv[]) {
    LsOptions ls_options = {};
    ls_options.term_width = get_terminal_width();

    int res = parse_arguments(argc, argv, &ls_options);
    if (res > 0) {
        return 0;
    }
    if (res < 0) {
        return 1;
    }

    char cwd[MAX_PATH];
    if (ls_options.target_directory != NULL) {
        strncpy(cwd, ls_options.target_directory, sizeof(cwd) - 1);
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

    int max_entry_len = 0;
    struct dirent *directory_entry;
    while ((directory_entry = readdir(directory)) != NULL) {
        if (ls_options.almost_all) {
            if (strcmp(directory_entry->d_name, ".") == 0 || strcmp(directory_entry->d_name, "..") == 0) {
                continue;
            }
        } else if (!ls_options.all && directory_entry->d_name[0] == '.') {
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

        entries[count] = malloc(sizeof(char) * strlen(directory_entry->d_name) + 1);
        if (!entries[count]) {
            for (int i = 0; i < count; i++) {
                free(entries[i]);
            }
            free(entries);

            closedir(directory);
            return 2;
        }

        int entry_len = (int)strlen(directory_entry->d_name);
        if (entry_len > max_entry_len) {
            max_entry_len = entry_len;
        }
        strcpy(entries[count], directory_entry->d_name);
        count++;
    }
    closedir(directory);

    if (!ls_options.nosort) {
        qsort(entries, count, sizeof(char *), cmp_entries);
        if (ls_options.reverse) {
            for (int i = 0; i < count / 2; i++) {
                char *tmp = entries[i];
                entries[i] = entries[count - i - 1];
                entries[count - i - 1] = tmp;
            }
        }
    }

    int col_width = max_entry_len + 2;
    int num_cols = ls_options.term_width / col_width;
    if (num_cols == 0) {
        num_cols = 1;
    }
    print_columns(entries, count, col_width, num_cols);

    for (int i = 0; i < count; i++) {
        free(entries[i]);
        entries[i] = NULL;
    }
    free(entries);
    entries = NULL;

    return 0;
}