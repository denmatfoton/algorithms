/**
 * @file test_utils.h
 * @brief Auxiliary functions for testing.
 */
#pragma once

#include <ctime>

double timespec_subtract(struct timespec &x, struct timespec &y, struct timespec &diff);
