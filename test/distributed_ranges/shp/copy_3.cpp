// -*- C++ -*-
//===----------------------------------------------------------------------===//
//
// Copyright (C) Intel Corporation
//
// SPDX-License-Identifier: Apache-2.0 WITH LLVM-exception
//
// This file incorporates work covered by the following copyright and permission
// notice:
//
// Part of the LLVM Project, under the Apache License v2.0 with LLVM Exceptions.
// See https://llvm.org/LICENSE.txt for license information.
//
//===----------------------------------------------------------------------===//

#include "copy.hpp"

TYPED_TEST_SUITE(CopyTest, AllocatorTypes);

TYPED_TEST(CopyTest, tests_from_this_file_run_on_3_devices) {
  EXPECT_EQ(dr::shp::nprocs(), 3);
  EXPECT_EQ(stdrng::size(dr::shp::devices()), 3);
}

TYPED_TEST(CopyTest, dist2local_wholesegment) {
  // when running on 3 devices copy exactly one segment
  const typename TestFixture::DistVec dist_vec = {1, 2, 3, 4,  5,  6,
                                                  7, 8, 9, 10, 11, 12};
  typename TestFixture::LocalVec local_vec = {0, 0, 0, 0};

  auto ret_it = dr::shp::copy(stdrng::begin(dist_vec) + 4,
                              stdrng::begin(dist_vec) + 8, stdrng::begin(local_vec));
  EXPECT_TRUE(gtest_equal(local_vec, typename TestFixture::LocalVec{5, 6, 7, 8}));
  EXPECT_EQ(ret_it, stdrng::end(local_vec));
}

TYPED_TEST(CopyTest, local2dist_wholesegment) {
  // when running on 3 devices copy into exactly one segment
  const typename TestFixture::LocalVec local_vec = {50, 60, 70, 80};
  typename TestFixture::DistVec dist_vec = {1, 2, 3, 4,  5,  6,
                                            7, 8, 9, 10, 11, 12};
  auto ret_it = dr::shp::copy(stdrng::begin(local_vec), stdrng::end(local_vec),
                              stdrng::begin(dist_vec) + 4);
  EXPECT_TRUE(gtest_equal(dist_vec, typename TestFixture::LocalVec{
                                  1, 2, 3, 4, 50, 60, 70, 80, 9, 10, 11, 12}));
  EXPECT_EQ(*ret_it, 9);
}
