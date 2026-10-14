/*
 * PROJECT:     ReactOS kernel-mode tests
 * LICENSE:     MIT (https://spdx.org/licenses/MIT)
 * PURPOSE:     Kernel mode tests for device power state (user part)
 * COPYRIGHT:   Copyright 2026 George Bișoc <george.bisoc@reactos.org>
 */

#include <kmt_test.h>
#include "po.h"

START_TEST(PoPowerState)
{
    DWORD Error;

    Error = KmtLoadAndOpenDriver(L"PoPowerState", TRUE);
    ok_eq_int(Error, ERROR_SUCCESS);
    if (Error)
        return;

    Error = KmtSendToDriver(IOCTL_RUN_TEST);
    ok(Error == ERROR_SUCCESS, "KmtSendUlongToDriver failed: %lx\n", Error);

    KmtCloseDriver();
    KmtUnloadDriver();
}
