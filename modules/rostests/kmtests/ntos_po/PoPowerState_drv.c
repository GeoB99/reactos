/*
 * PROJECT:     ReactOS kernel-mode tests
 * LICENSE:     MIT (https://spdx.org/licenses/MIT)
 * PURPOSE:     Kernel mode tests for device power state
 * COPYRIGHT:   Copyright 2026 George Bișoc <george.bisoc@reactos.org>
 */

#include <kmt_test.h>
#include <ntifs.h>

#include "po.h"

static KMT_MESSAGE_HANDLER TestMessageHandler;

#define DOE_DEVICE_POWER_FLAG_BIT 0xF0

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

    *DeviceName = L"PoPowerState";
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
DEVICE_POWER_STATE
GetDevicePowerState(
    _In_ PDEVICE_OBJECT DeviceObject)
{
    DEVICE_POWER_STATE DeviceState;
    PEXTENDED_DEVOBJ_EXTENSION DevObjExts;

    DevObjExts = ((PEXTENDED_DEVOBJ_EXTENSION)((DeviceObject)->DeviceObjectExtension)); // Taken from IoGetDevObjExtension
    DeviceState = ((DevObjExts->PowerFlags & DOE_DEVICE_POWER_FLAG_BIT) >> 4);
    return DeviceState;
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
    DEVICE_POWER_STATE DeviceState;
    POWER_STATE PrevState, NewState;

    PAGED_CODE();

    switch (ControlCode)
    {
        case IOCTL_RUN_TEST:
        {
            /* Put the driver in D0 state, make sure the state has changed */
            NewState.DeviceState = PowerDeviceD0;
            ok_irql(PASSIVE_LEVEL);
            PrevState = PoSetPowerState(DeviceObject, DevicePowerState, NewState);
            ok(PrevState.DeviceState != PowerDeviceD0, "Failed to change power state of device, previous state: %d\n", PrevState.DeviceState);

            DeviceState = GetDevicePowerState(DeviceObject);
            ok(DeviceState == PowerDeviceD0, "The power state of device must have been D0 but got %d\n", DeviceState);

            /* Now put it to D2 */
            NewState.DeviceState = PowerDeviceD2;
            PrevState = PoSetPowerState(DeviceObject, DevicePowerState, NewState);
            ok(PrevState.DeviceState != PowerDeviceD2, "Failed to change power state of device, previous state: %d\n", PrevState.DeviceState);

            DeviceState = GetDevicePowerState(DeviceObject);
            ok(DeviceState == PowerDeviceD2, "The power state of device must have been D2 but got %d\n", DeviceState);

            /* Finally put it to D3 */
            NewState.DeviceState = PowerDeviceD3;
            PrevState = PoSetPowerState(DeviceObject, DevicePowerState, NewState);
            ok(PrevState.DeviceState != PowerDeviceD3, "Failed to change power state of device, previous state: %d\n", PrevState.DeviceState);

            DeviceState = GetDevicePowerState(DeviceObject);
            ok(DeviceState == PowerDeviceD3, "The power state of device must have been D3 but got %d\n", DeviceState);
            break;
        }

        default:
            return STATUS_NOT_SUPPORTED;
    }

    return STATUS_SUCCESS;
}
