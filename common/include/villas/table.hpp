/* Print fancy tables.
 *
 * Author: Steffen Vogel <post@steffenvogel.de>
 * SPDX-FileCopyrightText: 2014-2023 Institute for Automation of Complex Power Systems, RWTH Aachen University
 * SPDX-License-Identifier: Apache-2.0
 */

#pragma once

#include <string>
#include <vector>

#include <fmt/printf.h>

#include <villas/log.hpp>

namespace villas {

class Table;

class TableColumn {

  friend Table;

public:
  enum class Alignment { LEFT, RIGHT };

protected:
  int _width; // The real width of this column. Calculated by Table::resize().

  int width;     // Width of the column.
  int precision; // Precision of the column, used for floating point values.

public:
  TableColumn(int w, int p, enum Alignment a, const std::string &t,
              const std::string &f, const std::string &u = "")
      : _width(0), width(w), precision(p), title(t), format(f), unit(u),
        align(a) {}

  std::string title;  // The title as shown in the table header.
  std::string format; // The format which is used to print the table rows.
  std::string unit; // An optional unit which will be shown in the table header.

  enum Alignment align;

  int getWidth() const { return _width; }
};

class Table {

protected:
  int resize(int w);
  void updateRowFormat();

  int width;
  std::vector<TableColumn> columns;
  std::string rowFormat;

  Logger logger;

public:
  Table(Logger log, const std::vector<TableColumn> &cols)
      : width(-1), columns(cols), logger(log) {}

  // Print a table header consisting of \p n columns.
  void header();

  // Print table rows.
  template <class... Args> void row(const Args &...args) {
    auto logWidth = Log::getInstance().getWidth();
    if (width != logWidth) {
      resize(logWidth);
      header();
    }

    logger->info(fmt::runtime(rowFormat), args...);
  }

  int getWidth() const { return width; }
};

} // namespace villas
