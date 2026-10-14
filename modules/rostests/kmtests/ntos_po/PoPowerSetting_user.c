/*
 * PROJECT:     ReactOS kernel-mode tests
 * LICENSE:     MIT (https://spdx.org/licenses/MIT)
 * PURPOSE:     Kernel mode tests for power setting callbacks (user part)
 * COPYRIGHT:   Copyright 2026 George Bișoc <george.bisoc@reactos.org>
 */

#include <kmt_test.h>
#include "Po.h"

START_TEST(PoPowerSetting)
{
    DWORD Error;

    Error = KmtLoadAndOpenDriver(L"PoPowerSetting", TRUE);
    ok_eq_int(Error, ERROR_SUCCESS);
    if (Error)
        return;

    Error = KmtSendToDriver(IOCTL_RUN_TEST);
    ok(Error == ERROR_SUCCESS, "KmtSendUlongToDriver failed: %lx\n", Error);
    if (Error)
        return;

    Error = KmtSendToDriver(IOCTL_FINISH_TEST);
    ok(Error == ERROR_SUCCESS, "KmtSendUlongToDriver failed: %lx\n", Error);

    KmtCloseDriver();
    KmtUnloadDriver();
}
