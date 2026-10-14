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

#include "po.h"

static KMT_MESSAGE_HANDLER TestMessageHandler;
static PVOID BatteryPercentageCallbackHandle = NULL;
static PVOID PowerSourceCallbackHandle = NULL;
static PVOID LidStateCallbackHandle = NULL;

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
    ULONG BatteryCapacity, LidState;
    ULONG BatteryChargeContext, LidStateContext;
    SYSTEM_POWER_CONDITION PowerCondition;
    SYSTEM_POWER_CONDITION PowerConditionContext;

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
    else if (CompareGuids(SettingGuid, &GUID_ACDC_POWER_SOURCE))
    {
        ok(ValueLength == sizeof(SYSTEM_POWER_CONDITION), "ValueLength is not of sizeof(SYSTEM_POWER_CONDITION) in GUID_ACDC_POWER_SOURCE\n");

        ok(Value != NULL, "Value is NULL in GUID_ACDC_POWER_SOURCE\n");
        PowerCondition = *((PULONG)Value);

        ok(Context != NULL, "Context is NULL in GUID_ACDC_POWER_SOURCE\n");
        PowerConditionContext = *((PULONG)Context);

        ok_eq_int(PowerCondition, PowerConditionContext);
    }
    else if (CompareGuids(SettingGuid, &GUID_LIDSWITCH_STATE_CHANGE))
    {
        ok(ValueLength == sizeof(ULONG), "ValueLength is not of sizeof(ULONG) in GUID_LIDSWITCH_STATE_CHANGE\n");

        ok(Value != NULL, "Value is NULL in GUID_LIDSWITCH_STATE_CHANGE\n");
        LidState = *((PULONG)Value);

        ok(Context != NULL, "Context is NULL in GUID_LIDSWITCH_STATE_CHANGE\n");
        LidStateContext = *((PULONG)Context);

        ok_eq_int(LidState, LidStateContext);
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
    _In_opt_ PVOID Buffer,
    _In_ SIZE_T InLength,
    _Inout_ PSIZE_T OutLength)
{
    ULONG FullCharge = 100;
    SYSTEM_POWER_CONDITION PowerCondition = PoAc;
    ULONG LidOpened = 1;
    NTSTATUS Status;

    PAGED_CODE();

    switch (ControlCode)
    {
        case IOCTL_RUN_TEST:
        {
            pPoRegisterPowerSettingCallback = KmtGetSystemRoutineAddress(L"PoRegisterPowerSettingCallback");
            if (!pPoRegisterPowerSettingCallback)
            {
                return STATUS_NOT_SUPPORTED;
            }

            pPoUnregisterPowerSettingCallback = KmtGetSystemRoutineAddress(L"PoUnregisterPowerSettingCallback");

            /*
             * Register 3 power callbacks and give them the context values the power manager should by
             * default set them upon initial call of the power setting callback.
             *
             * OBSERVATION: MS loves to write documentation but doesn't abide by them. On MSDN it's stated that
             * the power manager will initialize the power setting of the driver by immediately calling the callback
             * before PoRegisterPowerSettingCallback returns. This actually never happens here, instead ROS follows
             * the behavior as described in the documentation. This needs to be investigated in the future.
             */
            Status = RegisterPowerCallback(DeviceObject, &GUID_BATTERY_PERCENTAGE_REMAINING, &FullCharge, &BatteryPercentageCallbackHandle);
            ok(BatteryPercentageCallbackHandle != NULL, "BatteryPercentageCallbackHandle is NULL\n");
            ok_eq_hex(Status, STATUS_SUCCESS);

            Status = RegisterPowerCallback(DeviceObject, &GUID_ACDC_POWER_SOURCE, &PowerCondition, &PowerSourceCallbackHandle);
            ok(PowerSourceCallbackHandle != NULL, "PowerSourceCallbackHandle is NULL\n");
            ok_eq_hex(Status, STATUS_SUCCESS);

            Status = RegisterPowerCallback(DeviceObject, &GUID_LIDSWITCH_STATE_CHANGE, &LidOpened, &LidStateCallbackHandle);
            ok(LidStateCallbackHandle != NULL, "LidStateCallbackHandle is NULL\n");
            ok_eq_hex(Status, STATUS_SUCCESS);
            break;
        }

        case IOCTL_FINISH_TEST:
        {
            Status = UnregisterPowerCallback(LidStateCallbackHandle);
            ok_eq_hex(Status, STATUS_SUCCESS);

            Status = UnregisterPowerCallback(PowerSourceCallbackHandle);
            ok_eq_hex(Status, STATUS_SUCCESS);

            Status = UnregisterPowerCallback(BatteryPercentageCallbackHandle);
            ok_eq_hex(Status, STATUS_SUCCESS);
            break;
        }

        default:
            return STATUS_NOT_SUPPORTED;
    }

    return Status;
}
