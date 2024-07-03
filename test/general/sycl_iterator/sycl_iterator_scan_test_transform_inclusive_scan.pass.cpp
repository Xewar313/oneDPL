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

#include "sycl_iterator_test.h"

#if TEST_DPCPP_BACKEND_PRESENT

DEFINE_TEST(test_transform_inclusive_scan)
{
    DEFINE_TEST_CONSTRUCTOR(test_transform_inclusive_scan, 2.0f, 0.65f)

    template <typename Policy, typename Iterator1, typename Iterator2, typename Size>
    void
    operator()(Policy&& exec, Iterator1 first1, Iterator1 last1, Iterator2 first2, Iterator2 last2, Size n)
    {
        TestDataTransfer<UDTKind::eKeys, Size> host_keys(*this, n);
        TestDataTransfer<UDTKind::eVals, Size> host_vals(*this, n);

        typedef typename ::std::iterator_traits<Iterator1>::value_type T1;
        auto value = T1(333);

        ::std::fill(host_keys.get(), host_keys.get() + n, T1(1));
        host_keys.update_data();

        auto res1 = ::std::transform_inclusive_scan(
            make_new_policy<new_kernel_name<Policy, 0>>(exec), first1, last1, first2, ::std::plus<T1>(),
            [](T1 x) { return x * 2; }, value);
        wait_and_throw(exec);

        EXPECT_TRUE(res1 == last2, "wrong result from transform_inclusive_scan_1");

        retrieve_data(host_keys, host_vals);

        T1 ii = value;
        for (int i = 0; i < last2 - first2; ++i)
        {
            ii += 2 * host_keys.get()[i];
            if (host_vals.get()[i] != ii)
            {
                ::std::cout << "Error in scan_1: i = " << i << ", expected " << ii << ", got " << host_vals.get()[i]
                            << ::std::endl;
            }
            EXPECT_TRUE(host_vals.get()[i] == ii, "wrong effect from transform_inclusive_scan_1");
        }

        // without initial value
        auto res2 = ::std::transform_inclusive_scan(make_new_policy<new_kernel_name<Policy, 1>>(exec), first1, last1,
                                                    first2, ::std::plus<T1>(), [](T1 x) { return x * 2; });
        EXPECT_TRUE(res2 == last2, "wrong result from transform_inclusive_scan_2");

        retrieve_data(host_keys, host_vals);

        ii = 0;
        for (int i = 0; i < last2 - first2; ++i)
        {
            ii += 2 * host_keys.get()[i];
            if (host_vals.get()[i] != ii)
            {
                ::std::cout << "Error in scan_2: i = " << i << ", expected " << ii << ", got " << host_vals.get()[i]
                            << ::std::endl;
            }
            EXPECT_TRUE(host_vals.get()[i] == ii, "wrong effect from transform_inclusive_scan_2");
        }
    }
};

template <sycl::usm::alloc alloc_type>
void
test_usm_and_buffer()
{
    using ValueType = ::std::int32_t;

    PRINT_DEBUG("test_transform_inclusive_scan");
    test2buffers<alloc_type, test_transform_inclusive_scan<ValueType>>();
}
#endif // TEST_DPCPP_BACKEND_PRESENT

std::int32_t
main()
{
    try
    {
#if TEST_DPCPP_BACKEND_PRESENT
        // Run tests for USM shared memory
        test_usm_and_buffer<sycl::usm::alloc::shared>();
        // Run tests for USM device memory
        test_usm_and_buffer<sycl::usm::alloc::device>();
#endif // TEST_DPCPP_BACKEND_PRESENT
    }
    catch (const ::std::exception& exc)
    {
        std::cout << "Exception: " << exc.what() << std::endl;
        return EXIT_FAILURE;
    }

    return TestUtils::done(TEST_DPCPP_BACKEND_PRESENT);
}
