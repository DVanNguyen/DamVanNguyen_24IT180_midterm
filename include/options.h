/**
 * @file options.h
 * @brief Command-line options definitions and parsing for ls(1).
 */

#ifndef OPTIONS_H
#define OPTIONS_H

#include "compat.h"

typedef enum {
    TIME_MODIFICATION = 0,  /* Default: modification time (mtime) */
    TIME_STATUS_CHANGE,     /* -c: status change time (ctime) */
    TIME_ACCESS             /* -u: access time (atime) */
} TimeField;

typedef struct Options {
    /* Directory and file inclusion */
    bool all;               /* -a: include entries beginning with '.' */
    bool all_except_dots;   /* -A: list all except '.' and '..' */

    /* Traversal & recursion */
    bool list_dir_as_file;  /* -d: list directories as plain files */
    bool recursive;         /* -R: recursively list subdirectories */

    /* Listing format */
    bool long_format;       /* -l: long listing */
    bool numeric_ids;       /* -n: numeric UID/GID (-l implied) */
    bool inode;             /* -i: print file inode number */
    bool display_blocks;    /* -s: display block count */
    bool classify;          /* -F: append type indicator (/ * @ % = |) */
    bool human_readable;    /* -h: human-readable sizes for -s and -l */
    bool kilobytes;         /* -k: sizes reported in kilobytes for -s */

    /* Non-printable characters */
    bool non_printable_q;   /* -q: replace non-printable chars with '?' */
    bool raw_w;             /* -w: print raw non-printable chars */

    /* Sorting */
    bool unsorted;          /* -f: do not sort entries */
    bool reverse_sort;      /* -r: reverse sort order */
    bool sort_by_size;      /* -S: sort by size, largest first */
    bool sort_by_time;      /* -t: sort by time */
    TimeField time_field;   /* -c, -u, or default mtime */

    /* Environment & terminal info */
    long blocksize;         /* BLOCKSIZE env value (default 512) */
    bool is_terminal;       /* whether stdout is a terminal */

    /* File/Directory operands */
    char **operands;        /* Array of file/directory path arguments */
    int operand_count;      /* Number of operands */
} Options;

/**
 * @brief Initialize options structure with defaults.
 * @param opts Pointer to Options structure.
 */
void options_init(Options *opts);

/**
 * @brief Parse command-line arguments.
 * @param opts Pointer to Options structure.
 * @param argc Argument count.
 * @param argv Argument vector.
 * @return 0 on success, non-zero on error.
 */
int options_parse(Options *opts, int argc, char *argv[]);

/**
 * @brief Free allocated memory in Options structure.
 * @param opts Pointer to Options structure.
 */
void options_free(Options *opts);

/**
 * @brief Print usage information.
 * @param program_name Name of the executable.
 */
void options_print_usage(const char *program_name);

#endif /* OPTIONS_H */
