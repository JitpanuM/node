/* Histogram class.
 *
 * Author: Steffen Vogel <post@steffenvogel.de>
 * SPDX-FileCopyrightText: 2014-2023 Institute for Automation of Complex Power Systems, RWTH Aachen University
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>
#include <vector>

#include <jansson.h>

#include <villas/log.hpp>

#define HEIGHT (LOG_WIDTH - 55)
#define SEQ 17

namespace villas {

// Histogram structure used to collect statistics.
class Hist {

public:
  using cnt_t = uintmax_t;
  using idx_t = std::vector<cnt_t>::difference_type;

  // Initialize struct Hist with supplied values and allocate memory for buckets.
  Hist(int buckets = 0, cnt_t warmup = 0);

  // Reset all counters and values back to zero.
  void reset();

  // Count a value within its corresponding bucket.
  void put(double value);

  // Calculate the variance of all counted values.
  double getVar() const;

  // Calculate the mean average of all counted values.
  double getMean() const;

  // Calculate the standard derivation of all counted values.
  double getStddev() const;

  // Print all statistical properties of distribution including a graphical plot of the histogram.
  void print(Logger logger, bool details, std::string prefix = "") const;

  // Print ASCII style plot of histogram.
  void plot(Logger logger) const;

  // Dump histogram data in Matlab format.
  //
  // @return The string containing the dump. The caller is responsible to free() the buffer.
  char *dump() const;

  // Prints Matlab struct containing all infos to file.
  int dumpMatlab(FILE *f) const;

  // Write the histogram in JSON format to file \p f.
  int dumpJson(FILE *f) const;

  std::string toPrometheusText(const std::string &metric_name,
                               const std::string &node_name) const;

  // Build a libjansson / JSON object of the histogram.
  json_t *toJson() const;

  double getHigh() const { return high; }

  double getLow() const { return low; }

  double getHighest() const { return highest; }

  double getLowest() const { return lowest; }

  double getLast() const { return last; }

  cnt_t getTotal() const { return total; }

protected:
  double resolution; // The distance between two adjacent buckets.

  double high; // The value of the highest bucket.
  double low;  // The value of the lowest bucket.

  double highest; // The highest value observed (may be higher than #high).
  double lowest;  // The lowest value observed (may be lower than #low).
  double last;    // The last value which has been put into the buckets.

  cnt_t total;  // Total number of counted values.
  cnt_t warmup; // Number of values which are used during warmup phase.

  cnt_t higher; // The number of values which are higher than #high.
  cnt_t lower;  // The number of values which are lower than #low.

  std::vector<cnt_t> data; // Bucket counters.

  double _m[2], _s[2]; // Private variables for online variance calculation.
};

} // namespace villas
