/*
 * PROJECT:     ReactOS kernel-mode tests
 * LICENSE:     MIT (https://spdx.org/licenses/MIT)
 * PURPOSE:     Kernel mode tests for power request objects (user part)
 * COPYRIGHT:   Copyright 2026 George Bișoc <george.bisoc@reactos.org>
 */

#include <kmt_test.h>
#include "po.h"

START_TEST(PoPowerRequest)
{
    DWORD Error;

    Error = KmtLoadAndOpenDriver(L"PoPowerRequest", TRUE);
    ok_eq_int(Error, ERROR_SUCCESS);
    if (Error)
        return;

    Error = KmtSendToDriver(IOCTL_RUN_TEST);
    if (skip(Error != ERROR_NOT_SUPPORTED, "PoXxxPowerRequest are only available on Windows 7 and above.\n"))
        return;

    ok(Error == ERROR_SUCCESS, "KmtSendUlongToDriver failed: %lx\n", Error);
    if (Error)
        return;

    Error = KmtSendToDriver(IOCTL_FINISH_TEST);
    ok(Error == ERROR_SUCCESS, "KmtSendUlongToDriver failed: %lx\n", Error);

    KmtCloseDriver();
    KmtUnloadDriver();
}
