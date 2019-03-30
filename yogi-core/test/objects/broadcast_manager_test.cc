/*
 * This file is part of the Yogi distribution https://github.com/yohummus/yogi.
 * Copyright (c) 2018 Johannes Bergmann.
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, version 3.
 *
 * This library is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library. If not, see <http://www.gnu.org/licenses/>.
 */

#include "../common.h"

class BroadcastManagerTest : public TestFixture {
 protected:
  virtual void SetUp() override {
    context_ = CreateContext();
    branch_a_ = CreateBranch(context_);
    branch_b_ = CreateBranch(context_);
    branch_c_ = CreateBranch(context_);
  }

  void* context_;
  void* branch_a_;
  void* branch_b_;
  void* branch_c_;
};

TEST_F(BroadcastManagerTest, DISABLED_Bla) {

}
