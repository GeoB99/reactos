/*
 * PROJECT:     ReactOS kernel-mode tests
 * LICENSE:     MIT (https://spdx.org/licenses/MIT)
 * PURPOSE:     Kernel mode tests for device idle detection
 * COPYRIGHT:   Copyright 2026 George Bișoc <george.bisoc@reactos.org>
 */

#include <kmt_test.h>
#include <ntifs.h>

#include "po.h"

static KMT_MESSAGE_HANDLER TestMessageHandler;
static KMT_IRP_HANDLER TestIrpHandler;

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

    *DeviceName = L"PoIdle";
    *Flags = TESTENTRY_NO_EXCLUSIVE_DEVICE;

    KmtRegisterMessageHandler(0, NULL, TestMessageHandler);
    KmtRegisterIrpHandler(IRP_MJ_POWER, NULL, TestIrpHandler);

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
PULONG
RegisterDeviceForIdleDetection(
    _In_ PDEVICE_OBJECT DeviceObject)
{
    PULONG IdleCounter;

    ok_irql(PASSIVE_LEVEL);
    IdleCounter = PoRegisterDeviceForIdleDetection(DeviceObject,
                                                   3,
                                                   3,
                                                   PowerDeviceD2);
    return IdleCounter;
}

static
NTSTATUS
TestIrpHandler(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp,
    _In_ PIO_STACK_LOCATION IoStackLocation)
{
    ok(Irp != NULL, "Irp is NULL\n");
    ok_eq_uint(IoStackLocation->MajorFunction, IRP_MJ_POWER);
    ok_eq_uint(IoStackLocation->MinorFunction, IRP_MN_SET_POWER);
    ok_eq_uint(IoStackLocation->Parameters.Power.State.DeviceState, PowerDeviceD2);
    return STATUS_SUCCESS;
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
    NTSTATUS Status = STATUS_SUCCESS;
    PULONG IdleCounter;

    PAGED_CODE();

    switch (ControlCode)
    {
        case IOCTL_RUN_TEST:
        {
            IdleCounter = RegisterDeviceForIdleDetection(DeviceObject);
            ok(IdleCounter != NULL, "IdleCounter is NULL\n");
            if (IdleCounter == NULL)
                Status = STATUS_UNSUCCESSFUL;
            break;
        }

        case IOCTL_FINISH_TEST:
        {
            IdleCounter = PoRegisterDeviceForIdleDetection(DeviceObject, 0, 0, PowerDeviceUnspecified);
            ok(IdleCounter == NULL, "IdleCounter is not NULL\n");
            break;
        }

        default:
            return STATUS_NOT_SUPPORTED;
    }

    return Status;
}
