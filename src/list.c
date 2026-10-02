/**
 * @file list.c
 * @brief Implementation of directory traversal, sorting, and display.
 */

#include "list.h"
#include "sort.h"
#include "format.h"
#include "utils.h"

static bool s_has_printed_output = false;

static bool should_include_entry(const char *name, const Options *opts) {
    if (!name || !opts) return false;

    if (name[0] == '.') {
        if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) {
            return opts->all;
        }
        return opts->all || opts->all_except_dots;
    }

    return true;
}

int list_directory(const char *dir_path, const Options *opts, bool print_header, bool is_first) {
    if (!dir_path || !opts) return 1;

    DIR *dp = opendir(dir_path);
    if (!dp) {
        fprintf(stderr, "ls: %s: %s\n", dir_path, strerror(errno));
        return 1;
    }

    if (print_header) {
        if (!is_first || s_has_printed_output) {
            putchar('\n');
        }
        printf("%s:\n", dir_path);
        s_has_printed_output = true;
    }

    /* Initial capacity for directory entries */
    int capacity = 64;
    int count = 0;
    FileEntry **entries = malloc(sizeof(FileEntry *) * capacity);
    if (!entries) {
        closedir(dp);
        fprintf(stderr, "ls: out of memory\n");
        return 1;
    }

    struct dirent *de;
    while ((de = readdir(dp)) != NULL) {
        if (!should_include_entry(de->d_name, opts)) {
            continue;
        }

        FileEntry *fe = entry_create(dir_path, de->d_name, opts, false);
        if (!fe) continue;

        if (count >= capacity) {
            capacity *= 2;
            FileEntry **new_entries = realloc(entries, sizeof(FileEntry *) * capacity);
            if (!new_entries) {
                entry_free(fe);
                break;
            }
            entries = new_entries;
        }
        entries[count++] = fe;
    }
    closedir(dp);

    /* Sort entries */
    sort_entries(entries, count, opts);

    /* Print total blocks for directory if required */
    format_print_total(entries, count, opts);

    /* Print all entries */
    format_print_entries(entries, count, opts);

    if (count > 0 || print_header) {
        s_has_printed_output = true;
    }

    int exit_status = 0;

    /* Handle recursive listing (-R) */
    if (opts->recursive) {
        for (int i = 0; i < count; i++) {
            FileEntry *fe = entries[i];
            if (fe && fe->stat_ok && S_ISDIR(fe->sb.st_mode)) {
                if (strcmp(fe->name, ".") != 0 && strcmp(fe->name, "..") != 0) {
                    int sub_res = list_directory(fe->full_path, opts, true, false);
                    if (sub_res != 0) {
                        exit_status = 1;
                    }
                }
            }
        }
    }

    /* Free entries */
    for (int i = 0; i < count; i++) {
        entry_free(entries[i]);
    }
    free(entries);

    return exit_status;
}

int list_operands(const Options *opts) {
    if (!opts) return 1;

    s_has_printed_output = false;

    /* Case 1: No operands specified -> default to current directory */
    if (opts->operand_count == 0) {
        if (opts->list_dir_as_file) {
            FileEntry *dot = entry_create(NULL, ".", opts, false);
            if (!dot) return 1;
            FileEntry *arr[1] = { dot };
            format_print_entries(arr, 1, opts);
            entry_free(dot);
            return 0;
        }
        return list_directory(".", opts, false, true);
    }

    /* Case 2: One or more operands given */
    FileEntry **file_entries = malloc(sizeof(FileEntry *) * opts->operand_count);
    FileEntry **dir_entries = malloc(sizeof(FileEntry *) * opts->operand_count);
    if (!file_entries || !dir_entries) {
        if (file_entries) free(file_entries);
        if (dir_entries) free(dir_entries);
        fprintf(stderr, "ls: out of memory\n");
        return 1;
    }

    int file_count = 0;
    int dir_count = 0;
    int exit_status = 0;

    for (int i = 0; i < opts->operand_count; i++) {
        const char *op = opts->operands[i];
        struct stat st;

        if (compat_lstat(op, &st) != 0) {
            fprintf(stderr, "ls: %s: %s\n", op, strerror(errno));
            exit_status = 1;
            continue;
        }

        if (opts->list_dir_as_file) {
            FileEntry *fe = entry_create(NULL, op, opts, false);
            if (fe) file_entries[file_count++] = fe;
            continue;
        }

        if (S_ISDIR(st.st_mode)) {
            FileEntry *fe = entry_create(NULL, op, opts, false);
            if (fe) dir_entries[dir_count++] = fe;
        } else if (S_ISLNK(st.st_mode)) {
            /* If -l or -F is specified, treat symlink operand as file */
            if (opts->long_format || opts->classify) {
                FileEntry *fe = entry_create(NULL, op, opts, false);
                if (fe) file_entries[file_count++] = fe;
            } else {
                /* Otherwise check if symlink target is a directory */
                struct stat target_st;
                if (stat(op, &target_st) == 0 && S_ISDIR(target_st.st_mode)) {
                    FileEntry *fe = entry_create(NULL, op, opts, false);
                    if (fe) dir_entries[dir_count++] = fe;
                } else {
                    FileEntry *fe = entry_create(NULL, op, opts, false);
                    if (fe) file_entries[file_count++] = fe;
                }
            }
        } else {
            FileEntry *fe = entry_create(NULL, op, opts, false);
            if (fe) file_entries[file_count++] = fe;
        }
    }

    /* Sort non-directory operands */
    sort_entries(file_entries, file_count, opts);

    /* Sort directory operands */
    sort_entries(dir_entries, dir_count, opts);

    /* Print non-directory operands first */
    if (file_count > 0) {
        format_print_entries(file_entries, file_count, opts);
        s_has_printed_output = true;
    }

    /* Then print directory operands */
    bool print_hdr = (opts->operand_count > 1 || file_count > 0 || opts->recursive);
    for (int i = 0; i < dir_count; i++) {
        bool is_first = (file_count == 0 && i == 0);
        int res = list_directory(dir_entries[i]->full_path, opts, print_hdr, is_first);
        if (res != 0) {
            exit_status = 1;
        }
    }

    /* Clean up allocated FileEntry pointers */
    for (int i = 0; i < file_count; i++) {
        entry_free(file_entries[i]);
    }
    for (int i = 0; i < dir_count; i++) {
        entry_free(dir_entries[i]);
    }
    free(file_entries);
    free(dir_entries);

    return exit_status;
}
