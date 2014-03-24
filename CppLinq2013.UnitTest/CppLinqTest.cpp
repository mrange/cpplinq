// ----------------------------------------------------------------------------------------------
// Copyright (c) Mårten Rånge.
// ----------------------------------------------------------------------------------------------
// This source code is subject to terms and conditions of the Microsoft Public License. A
// copy of the license can be found in the License.html file at the root of this distribution.
// If you cannot locate the  Microsoft Public License, please send an email to
// dlr@microsoft.com. By using this source code in any fashion, you are agreeing to be bound
//  by the terms of the Microsoft Public License.
// ----------------------------------------------------------------------------------------------
// You must not remove this notice, or any other, from this software.
// ----------------------------------------------------------------------------------------------
#include "stdafx.h"
#include "CppUnitTest.h"
// ----------------------------------------------------------------------------------------------
// In order to suppress code coverage of assert macros
#define CPPLINQ_ASSERT(expr)
// ----------------------------------------------------------------------------------------------
#include "../Test/CppLinqTests.hpp"
// ----------------------------------------------------------------------------------------------
using namespace Microsoft::VisualStudio::CppUnitTestFramework;
// ----------------------------------------------------------------------------------------------
namespace CppLinq2013UnitTest
{
    TEST_CLASS(CppLinqTest)
    {
    public:

        TEST_METHOD(RunTestSuite)
        {
            auto failures_detected = run_all_tests (false);
            Assert::AreEqual (false, failures_detected);
        }

    };
}
// ----------------------------------------------------------------------------------------------
