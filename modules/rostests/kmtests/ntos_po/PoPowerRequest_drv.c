/*
 * PROJECT:     ReactOS kernel-mode tests
 * LICENSE:     MIT (https://spdx.org/licenses/MIT)
 * PURPOSE:     Kernel mode tests for power request objects
 * COPYRIGHT:   Copyright 2026 George Bișoc <george.bisoc@reactos.org>
 */

#include <kmt_test.h>
#include <ntifs.h>

#include "po.h"

static KMT_MESSAGE_HANDLER TestMessageHandler;
static PVOID PowerRequestHandle = NULL;

NTSTATUS
(NTAPI *pPoCreatePowerRequest)(
    _Out_ PVOID *PowerRequest,
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_opt_ PCOUNTED_REASON_CONTEXT Context);

NTSTATUS
(NTAPI *pPoClearPowerRequest)(
    _Inout_ PVOID PowerRequest,
    _In_ POWER_REQUEST_TYPE Type);

NTSTATUS
(NTAPI *pPoSetPowerRequest)(
    _Inout_ PVOID PowerRequest,
    _In_ POWER_REQUEST_TYPE Type);

NTSTATUS
(NTAPI *pPoDeletePowerRequest)(
    _Inout_ PVOID PowerRequest);

NTSTATUS
TestEntry(
    _In_ PDRIVER_OBJECT DriverObject,
    _In_ PCUNICODE_STRING RegistryPath,
    _Out_ PCWSTR *DeviceName,
    _Inout_ INT *Flags)
{
    PAGED_CODE();

    UNREFERENCED_PARAMETER(RegistryPath);
    UNREFERENCED_PARAMETER(DriverObject);

    *DeviceName = L"PoPowerRequest";
    *Flags = TESTENTRY_NO_EXCLUSIVE_DEVICE;

    KmtRegisterMessageHandler(0, NULL, TestMessageHandler);

    return STATUS_SUCCESS;
}

VOID
TestUnload(
    _In_ PDRIVER_OBJECT DriverObject)
{
    UNREFERENCED_PARAMETER(DriverObject);

    PAGED_CODE();
}

static
NTSTATUS
TestMessageHandler(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ ULONG ControlCode,
    _In_opt_ PVOID Buffer,
    _In_ SIZE_T InLength,
    _Inout_ PSIZE_T OutLength)
{
    COUNTED_REASON_CONTEXT Context;
    NTSTATUS Status;

    PAGED_CODE();

    switch (ControlCode)
    {
        case IOCTL_RUN_TEST:
        {
            pPoCreatePowerRequest = KmtGetSystemRoutineAddress(L"PoCreatePowerRequest");
            if (!pPoCreatePowerRequest)
            {
                return STATUS_NOT_SUPPORTED;
            }

            pPoClearPowerRequest = KmtGetSystemRoutineAddress(L"PoClearPowerRequest");
            pPoSetPowerRequest = KmtGetSystemRoutineAddress(L"PoSetPowerRequest");
            pPoDeletePowerRequest = KmtGetSystemRoutineAddress(L"PoDeletePowerRequest");

            /* Start with a simple reason for this power request and create the request */
            RtlZeroMemory(&Context, sizeof(Context));
            Context.Version = DIAGNOSTIC_REASON_VERSION;
            Context.Flags = DIAGNOSTIC_REASON_SIMPLE_STRING;
            RtlInitUnicodeString(&Context.SimpleString, L"Power request created by KMTEST for testing purposes.");

            ok_irql(PASSIVE_LEVEL);
            Status = pPoCreatePowerRequest(&PowerRequestHandle, DeviceObject, &Context);
            ok(PowerRequestHandle != NULL, "PowerRequestHandle is NULL\n");
            ok_eq_hex(Status, STATUS_SUCCESS);
            if (!NT_SUCCESS(Status))
                return Status;

            /*
             * Kernel mode drivers can only set PowerRequestSystemRequired,
             * the other types can only be set by user mode callers.
             */
            Status = pPoSetPowerRequest(PowerRequestHandle, PowerRequestDisplayRequired);
            ok_eq_hex(Status, STATUS_NOT_SUPPORTED);

            /* Tell the power manager this driver is making the system busy */
            Status = pPoSetPowerRequest(PowerRequestHandle, PowerRequestSystemRequired);
            ok_eq_hex(Status, STATUS_SUCCESS);
            break;
        }

        case IOCTL_FINISH_TEST:
        {
            /* Tell the power manager we are no longer bothering with the system anymore */
            Status = pPoClearPowerRequest(PowerRequestHandle, PowerRequestSystemRequired);
            ok_eq_hex(Status, STATUS_SUCCESS);

            Status = pPoDeletePowerRequest(PowerRequestHandle);
            ok_eq_hex(Status, STATUS_SUCCESS);
            break;
        }

        default:
            return STATUS_NOT_SUPPORTED;
    }

    return Status;
}
