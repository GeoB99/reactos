/*
 * PROJECT:     ReactOS kernel-mode tests
 * LICENSE:     MIT (https://spdx.org/licenses/MIT)
 * PURPOSE:     Kernel mode tests for power setting callbacks
 * COPYRIGHT:   Copyright 2026 George Bișoc <george.bisoc@reactos.org>
 */

#include <kmt_test.h>

#include <initguid.h>
#include <wdm.h>
#include <poclass.h>

#include "Po.h"

static KMT_MESSAGE_HANDLER TestMessageHandler;
PVOID BatteryPercentageCallbackHandle = NULL;

NTSTATUS
(NTAPI *pPoRegisterPowerSettingCallback)(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ LPCGUID SettingGuid,
    _In_ PPOWER_SETTING_CALLBACK Callback,
    _In_opt_ PVOID Context,
    _Out_ PVOID *Handle);

NTSTATUS
(NTAPI *pPoUnregisterPowerSettingCallback)(
    _Inout_ PVOID Handle);

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

    *DeviceName = L"PoPowerSetting";
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
BOOLEAN
CompareGuids(
    _In_ LPCGUID Guid1,
    _In_ LPCGUID Guid2)
{
    return (RtlCompareMemory(Guid1, Guid2, sizeof(GUID)) == sizeof(GUID));
}

static
NTSTATUS
NTAPI
TestPowerSettingCallback(
    _In_ LPCGUID SettingGuid,
    _In_reads_bytes_(ValueLength) PVOID Value,
    _In_ ULONG ValueLength,
    _Inout_opt_ PVOID Context)
{
    ULONG BatteryCapacity;
    ULONG BatteryChargeContext;

    ok_irql(PASSIVE_LEVEL);

    if (CompareGuids(SettingGuid, &GUID_BATTERY_PERCENTAGE_REMAINING))
    {
        ok(ValueLength == sizeof(ULONG), "ValueLength is not of sizeof(ULONG) in GUID_BATTERY_PERCENTAGE_REMAINING\n");

        ok(Value != NULL, "Value is NULL in GUID_BATTERY_PERCENTAGE_REMAINING\n");
        BatteryCapacity = *((PULONG)Value);

        ok(Context != NULL, "Context is NULL in GUID_BATTERY_PERCENTAGE_REMAINING\n");
        BatteryChargeContext = *((PULONG)Context);

        ok_eq_int(BatteryCapacity, BatteryChargeContext);
    }
    else
    {
        ok(0, "Unexpected GUID\n");
    }

    return STATUS_SUCCESS;
}

static
NTSTATUS
UnregisterPowerCallback(
    _In_ PVOID Handle)
{
    NTSTATUS Status;

    ok_irql(PASSIVE_LEVEL);
    Status = pPoUnregisterPowerSettingCallback(Handle);
    return Status;
}

static
NTSTATUS
RegisterPowerCallback(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ LPCGUID SettingGuid,
    _In_ PVOID Context,
    _Out_ PVOID *TestHandle)
{
    NTSTATUS Status;

    ok_irql(PASSIVE_LEVEL);
    Status = pPoRegisterPowerSettingCallback(DeviceObject,
                                             SettingGuid,
                                             TestPowerSettingCallback,
                                             Context,
                                             TestHandle);
    return Status;
}

static
NTSTATUS
TestMessageHandler(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ ULONG ControlCode,
    _In_ PVOID Buffer OPTIONAL,
    _In_ SIZE_T InLength,
    _Inout_ PSIZE_T OutLength)
{
    ULONG FullCharge = 100;
    NTSTATUS Status;

    PAGED_CODE();

    switch (ControlCode)
    {
        case IOCTL_RUN_TEST:
        {
            pPoRegisterPowerSettingCallback = KmtGetSystemRoutineAddress(L"PoRegisterPowerSettingCallback");
            if (skip(pPoRegisterPowerSettingCallback != NULL, "PoRegisterPowerSettingCallback is only available on Vista+.\n"))
            {
                return STATUS_NOT_SUPPORTED;
            }

            pPoUnregisterPowerSettingCallback = KmtGetSystemRoutineAddress(L"PoUnregisterPowerSettingCallback");
            if (skip(pPoUnregisterPowerSettingCallback != NULL, "PoUnregisterPowerSettingCallback is only available on Vista+.\n"))
            {
                return STATUS_NOT_SUPPORTED;
            }

            Status = UnregisterPowerCallback((PVOID)1);
            ok_eq_hex(Status, STATUS_INVALID_PARAMETER);

            Status = RegisterPowerCallback(DeviceObject, &GUID_BATTERY_PERCENTAGE_REMAINING, &FullCharge, &BatteryPercentageCallbackHandle);
            ok(BatteryPercentageCallbackHandle != NULL, "BatteryPercentageCallbackHandle is NULL\n");
            ok_eq_hex(Status, STATUS_SUCCESS);
            break;
        }

        case IOCTL_FINISH_TEST:
        {
            Status = UnregisterPowerCallback(BatteryPercentageCallbackHandle);
            ok_eq_hex(Status, STATUS_SUCCESS);
            break;
        }

        default:
            return STATUS_NOT_SUPPORTED;
    }

    return Status;
}
