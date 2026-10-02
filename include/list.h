/**
 * @file list.h
 * @brief Directory traversal, operand handling, and recursive listing.
 */

#ifndef LIST_H
#define LIST_H

#include "entry.h"
#include "options.h"

/**
 * @brief Process all operands and execute directory listing.
 * @param opts Program options.
 * @return 0 on success, >0 if any error occurred.
 */
int list_operands(const Options *opts);

/**
 * @brief List contents of a single directory.
 * @param dir_path Path to the directory.
 * @param opts Program options.
 * @param print_header Whether to print directory name header (e.g. "dir:").
 * @param is_first Whether this is the first output of the program.
 * @return 0 on success, >0 on error.
 */
int list_directory(const char *dir_path, const Options *opts, bool print_header, bool is_first);

#endif /* LIST_H */
