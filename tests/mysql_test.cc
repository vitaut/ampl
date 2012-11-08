/*
 Tests of the MySQL ODBC connection.

 Copyright (C) 2012 AMPL Optimization LLC

 Permission to use, copy, modify, and distribute this software and its
 documentation for any purpose and without fee is hereby granted,
 provided that the above copyright notice appear in all copies and that
 both that the copyright notice and this permission notice and warranty
 disclaimer appear in supporting documentation.

 The author and AMPL Optimization LLC disclaim all warranties with
 regard to this software, including all implied warranties of
 merchantability and fitness.  In no event shall the author be liable
 for any special, indirect or consequential damages or any damages
 whatsoever resulting from loss of use, data or profits, whether in an
 action of contract, negligence or other tortious action, arising out
 of or in connection with the use or performance of this software.

 Author: Victor Zverovich
 */

#include "gtest/gtest.h"
#include "tests/function.h"
#include "tests/odbc.h"

using fun::Table;

#define SERVER "callisto.local"

namespace {

class MySQLTest : public ::testing::Test {
 protected:
  static fun::Library lib_;

  static void SetUpTestCase() {
    lib_.Load();
  }

  void Read(Table *t) {
    lib_.GetHandler("odbc")->Read(t);
  }
};

fun::Library MySQLTest::lib_("../tables/ampltabl.dll");

TEST_F(MySQLTest, Read) {
  std::string driver_name(odbc::Env().FindDriver("mysql"));
  std::string connection(
      "DRIVER={" + driver_name + "}; SERVER=" SERVER "; DATABASE=test;");
  Table t("", "ODBC", connection.c_str(), "SQL=SELECT VERSION();");
  t.AddCol("VERSION()");
  Read(&t);
  EXPECT_EQ(1, t.num_rows());
  EXPECT_TRUE(t.GetString(0) != nullptr);
}

// TODO(viz): more tests
}
