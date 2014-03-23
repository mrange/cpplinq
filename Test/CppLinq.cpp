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
// ----------------------------------------------------------------------------------------------
#include "CppLinqTests.hpp"
// ----------------------------------------------------------------------------------------------
int main (int argc, char const * argv[])
{
    printf (
        "CppLinq test program\n"
        "====================\n"
        "Command line options:\n"
        "f - run functional tests (default)\n"
#ifndef _DEBUG
        "p - run functional + performance tests\n"
#endif
#ifdef _MSC_VER
#   ifdef _DEBUG
        "m - run memory leak detection test\n"
#   endif
#endif
        );

    auto opt = argc < 2 ? 'f' : argv[1][0];

    auto failures_detected = false;

    switch (opt)
    {
#ifdef _MSC_VER
#   ifdef _DEBUG
    case 'm':
        {
            printf ("Starting memory leak detection test...\n");

            _CrtMemState before;
            _CrtMemState after;
            _CrtMemState diff;

            _CrtMemCheckpoint (&before);

            run_all_tests (false);

            _CrtMemCheckpoint (&after);
            if (_CrtMemDifference(&diff, &before, &after))
            {
                auto reporter = [] (int nRptType, char *szMsg, int *retVal)
                {
                    *retVal = 0;

                    char const * rptType = "UNKNOWN";
                    switch (nRptType)
                    {
                    case _CRT_WARN:
                        rptType = "WARNING";
                        break;
                    case _CRT_ERROR:
                        rptType = "ERROR  ";
                        break;
                    case _CRT_ASSERT:
                        rptType = "ASSERT ";
                        break;
                    default:
                        break;
                    }

                    printf (
                            "%s - %s"
                        ,   rptType
                        ,   szMsg
                        );
                    return 0;
                };
                _CrtSetReportHook (reporter);

               _CrtMemDumpStatistics (&diff);

                printf ("Dumping leaked objects...\n");

                _CrtDumpMemoryLeaks ();

                printf (
                    "Objects dumped\n"
                    "Note: Some leaks are expected due to CRT allocatations\n"
                    );

                _CrtSetReportHook (nullptr);

                failures_detected = true;
            }
            else
            {
                failures_detected = false;
            }

            printf ("Memory leak detection finished\n");
        }
        break;
#   endif
#endif
#ifndef _DEBUG
    case 'p':
        printf ("Starting functional + performance tests...\n");

        failures_detected = run_all_tests (true);

        printf ("Functional + performance tests finished\n");
        break;
#endif
    case 'f':
    default:
        printf ("Starting functional tests...\n");

        failures_detected = run_all_tests (false);

        printf ("Functional tests finished\n");
        break;
    }

    if (failures_detected)
    {
        printf ("Tests failed, reporting 101\n");
        return 101;
    }
    else
    {
        printf ("Tests succeeded, reporting 0\n");
        return 0;
    }
}
// ---------------------------------------------------------------------------- -

