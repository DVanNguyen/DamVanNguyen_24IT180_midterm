/**
 * @file options.c
 * @brief Implementation of option parsing for ls(1).
 */

#include "options.h"

static long parse_blocksize(void) {
    const char *env = getenv("BLOCKSIZE");
    if (!env || *env == '\0') {
        return 512;
    }

    char *endptr = NULL;
    long val = strtol(env, &endptr, 10);
    if (val <= 0) {
        return 512;
    }

    if (endptr && *endptr != '\0') {
        char unit = *endptr;
        if (unit == 'k' || unit == 'K') {
            val *= 1024;
        } else if (unit == 'm' || unit == 'M') {
            val *= 1024 * 1024;
        } else if (unit == 'g' || unit == 'G') {
            val *= 1024 * 1024 * 1024;
        }
    }

    return (val > 0) ? val : 512;
}

void options_init(Options *opts) {
    if (!opts) return;
    memset(opts, 0, sizeof(*opts));

    opts->is_terminal = compat_is_stdout_terminal();
    if (opts->is_terminal) {
        opts->non_printable_q = true;
        opts->raw_w = false;
    } else {
        opts->raw_w = true;
        opts->non_printable_q = false;
    }

    opts->blocksize = parse_blocksize();
    opts->time_field = TIME_MODIFICATION;
}

void options_print_usage(const char *program_name) {
    const char *base = strrchr(program_name, PATH_SEPARATOR);
    if (base) {
        program_name = base + 1;
    }
    fprintf(stderr, "usage: %s [-AacdFfhiklnqRrSstuw] [file ...]\n", program_name);
}

int options_parse(Options *opts, int argc, char *argv[]) {
    if (!opts || argc < 1 || !argv) return 1;

    int i = 1;
    bool parsing_flags = true;

    /* Pre-allocate operands array with enough capacity */
    opts->operands = malloc(sizeof(char *) * (argc + 1));
    if (!opts->operands) {
        fprintf(stderr, "ls: out of memory\n");
        return 1;
    }
    opts->operand_count = 0;

    for (i = 1; i < argc; i++) {
        char *arg = argv[i];

        if (parsing_flags && arg[0] == '-' && arg[1] != '\0') {
            /* Check for end of flags delimiter '--' */
            if (arg[1] == '-' && arg[2] == '\0') {
                parsing_flags = false;
                continue;
            }

            /* Parse grouped short flags */
            for (int j = 1; arg[j] != '\0'; j++) {
                char opt = arg[j];
                switch (opt) {
                    case 'A':
                        opts->all_except_dots = true;
                        break;
                    case 'a':
                        opts->all = true;
                        break;
                    case 'c':
                        /* -c and -u override each other */
                        opts->time_field = TIME_STATUS_CHANGE;
                        break;
                    case 'd':
                        /* -R and -d override each other */
                        opts->list_dir_as_file = true;
                        opts->recursive = false;
                        break;
                    case 'F':
                        opts->classify = true;
                        break;
                    case 'f':
                        /* -f: output is not sorted, implies -a */
                        opts->unsorted = true;
                        opts->all = true;
                        break;
                    case 'h':
                        /* -h overrides -k */
                        opts->human_readable = true;
                        opts->kilobytes = false;
                        break;
                    case 'i':
                        opts->inode = true;
                        break;
                    case 'k':
                        /* -k overrides -h */
                        opts->kilobytes = true;
                        opts->human_readable = false;
                        break;
                    case 'l':
                        /* -l and -n override each other */
                        opts->long_format = true;
                        opts->numeric_ids = false;
                        break;
                    case 'n':
                        /* -n is -l with numeric UID/GID */
                        opts->long_format = true;
                        opts->numeric_ids = true;
                        break;
                    case 'q':
                        /* -q and -w override each other */
                        opts->non_printable_q = true;
                        opts->raw_w = false;
                        break;
                    case 'R':
                        /* -R and -d override each other */
                        opts->recursive = true;
                        opts->list_dir_as_file = false;
                        break;
                    case 'r':
                        opts->reverse_sort = true;
                        break;
                    case 'S':
                        opts->sort_by_size = true;
                        break;
                    case 's':
                        opts->display_blocks = true;
                        break;
                    case 't':
                        opts->sort_by_time = true;
                        break;
                    case 'u':
                        /* -c and -u override each other */
                        opts->time_field = TIME_ACCESS;
                        break;
                    case 'w':
                        /* -w and -q override each other */
                        opts->raw_w = true;
                        opts->non_printable_q = false;
                        break;
                    default:
                        fprintf(stderr, "ls: unknown option -- %c\n", opt);
                        options_print_usage(argv[0]);
                        return 1;
                }
            }
        } else {
            /* Position operand */
            opts->operands[opts->operand_count++] = arg;
        }
    }

    opts->operands[opts->operand_count] = NULL;
    return 0;
}

void options_free(Options *opts) {
    if (!opts) return;
    if (opts->operands) {
        free(opts->operands);
        opts->operands = NULL;
    }
    opts->operand_count = 0;
}
