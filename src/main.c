/**
 * @file main.c
 * @brief Entry point for the custom ls(1) utility.
 */

#include "compat.h"
#include "options.h"
#include "list.h"

int main(int argc, char *argv[]) {
    Options opts;
    options_init(&opts);

    if (options_parse(&opts, argc, argv) != 0) {
        options_free(&opts);
        return 1;
    }

    int exit_status = list_operands(&opts);

    options_free(&opts);
    return exit_status;
}
