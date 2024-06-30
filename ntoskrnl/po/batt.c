/*
 * PROJECT:     ReactOS Kernel
 * LICENSE:     GPL-2.0-or-later (https://spdx.org/licenses/GPL-2.0-or-later)
 * PURPOSE:     Composite battery power management routines
 * COPYRIGHT:   Copyright 2023 George Bișoc <george.bisoc@reactos.org>
 */

/* INCLUDES *******************************************************************/

#include <ntoskrnl.h>
#define NDEBUG
#include <debug.h>

/* GLOBALS ********************************************************************/

PPOP_BATTERY PopBattery;
KEVENT PopFreshBatteryDataEvent;

/* COMPLETION ROUTINE *********************************************************/

static IO_COMPLETION_ROUTINE PopBatteryIrpCompletion;

/* PRIVATE FUNCTIONS **********************************************************/

static
VOID
PopIsSystemDrainingAcSource(
    _In_ ULONG PowerState,
    _Out_ PBOOLEAN Changed)
{
    PSYSTEM_POWER_POLICY SystemPowerPolicy;

    /* Assume the power policy has not changed */
    *Changed = FALSE;
    SystemPowerPolicy = PopDefaultPowerPolicy;

    /*
     * A fully charged up battery is indicated when the machine drains
     * electrical power from the AC source and not from the battery itself.
     */
    if (PowerState & BATTERY_POWER_ON_LINE)
    {
        /* The system is indeed consuming power from AC source, set the AC policy */
        PopSetDefaultPowerPolicy(PolicyAc);
    }
    else
    {
        /* The system is no longer accessing power from AC source, set the DC policy */
        PopSetDefaultPowerPolicy(PolicyDc);
    }

    /*
     * Now check if the power policy has changed at all. If it did, it indicates
     * the event of the system draining AC power is occurring just now.
     */
    if (SystemPowerPolicy != PopDefaultPowerPolicy)
    {
        *Changed = TRUE;
    }
}

static
NTSTATUS
NTAPI
PopBatteryIrpCompletion(
    _In_ PDEVICE_OBJECT DeviceObject,
    _In_ PIRP Irp,
    _In_ PVOID Context)
{
    NTSTATUS Status;
    UCHAR Mode;
    ULONG Tag;
    ULONG EstTime;
    BATTERY_STATUS BattStatus;
    BATTERY_INFORMATION BattInfo;
    BOOLEAN PolicyChanged;
    PPOP_DEVICE_POLICY_WORKITEM_DATA WorkItemData;

    /* We do not care about these as we already have them in the composite battery */
    UNREFERENCED_PARAMETER(DeviceObject);
    UNREFERENCED_PARAMETER(Irp);

    /* Check if our I/O request has succeeded */
    Status = PopBattery->Irp->IoStatus.Status;
    if (!NT_SUCCESS(Status))
    {
        /*
         * It failed. However it could also be that the IRP has been cancelled
         * by the Battery Class driver of which we must re-issue the I/O request
         * from the beginning.
         */
        if (Status == STATUS_CANCELLED ||
            PopBattery->Irp->Cancel)
        {
            DPRINT1("IRP 0x%p has been CANCELLED, must re-issue a new I/O battery request\n", PopBattery->Irp);
            PopBattery->Flags |= POP_CB_RETRY_IO_REQUEST;
            IoFreeIrp(PopBattery->Irp);
            PopBattery->Irp = NULL;
            goto InvokeHandler;
        }

        /*
         * We have failed the request by other means. There is nothing that we can do
         * at this point. We must inquire the removal process of the battery and jump
         * back to AC power policy and such.
         */
        DPRINT1("The composite battery I/O request has failed (Status 0x%08lx)\n", Status);
        PopBattery->Flags |= POP_CB_REMOVE_BATTERY;
        goto InvokeHandler;
    }

    /* Process this request depending on what we actually asked */
    Mode = PopBattery->Mode;
    switch (Mode)
    {
        case POP_CB_READ_TAG_MODE:
        {
            /*
             * We have gotten the battery tag of which we can use it to identify
             * it and query critical battery data based on this tag. Take away
             * the processing mode request.
             */
            PopBattery->Flags &= ~POP_CB_PROCESSING_MODE_REQUEST;

            /* Copy the battery tag into the kernel structure */
            Tag = *(PULONG)Irp->AssociatedIrp.SystemBuffer;
            PopBattery->BatteryTag = Tag;

            /* The system now has a composite battery, take away the "no battery" flag */
            PopBattery->Flags &= ~POP_CB_NO_BATTERY;

            /* FIXME: Once I implement battery trigger levels, the CB levels must be reset here */

            /*
             * If this was a new battery candidate or the I/O operation had to be
             * retried because the composite battery driver cancelled our IRP, then
             * take away these flags.
             */
            if (PopBattery->Flags & POP_CB_PENDING_NEW_BATTERY)
            {
                PopBattery->Flags &= ~POP_CB_PENDING_NEW_BATTERY;
            }

            if (PopBattery->Flags & POP_CB_RETRY_IO_REQUEST)
            {
                PopBattery->Flags &= ~POP_CB_RETRY_IO_REQUEST;
            }

            break;
        }

        case POP_CB_QUERY_INFORMATION_MODE:
        {
            /*
             * We have the necessary battery information so that the battery
             * status can be queried later. Take away the processing mode request
             * and copy the information.
             */
            PopBattery->Flags &= ~POP_CB_PROCESSING_MODE_REQUEST;
            BattInfo = *(PBATTERY_INFORMATION)Irp->AssociatedIrp.SystemBuffer;
            RtlCopyMemory(&PopBattery->BattInfo, &BattInfo, sizeof(BattInfo));
            break;
        }

        case POP_CB_QUERY_STATUS_MODE:
        {
            /*
             * We have queried the current battery status. Take away the
             * processing mode request.
             */
            PopBattery->Flags &= ~POP_CB_PROCESSING_MODE_REQUEST;

            /*
             * Figure out if the current global power policy must be changed or
             * not, to DC or AC. We have to determine that by checking if the
             * machine is draining electrical power from the AC source and not
             * from the battery.
             */
            BattStatus = *(PBATTERY_STATUS)Irp->AssociatedIrp.SystemBuffer;
            PopIsSystemDrainingAcSource(BattStatus.PowerState, &PolicyChanged);

            /*
             * The system power policy has changed. We need to recompute the thermal throttle
             * gauges and the system idleness.
             *
             * FIXME: And of course code does not get written by itself.... a human can do that.
             */
            if (PolicyChanged)
            {
                DPRINT1("System power policy has been changed, recomputing system idleness and throttle not IMPLEMENTED yet\n");
            }

            /* Copy the currently queried battery status */
            RtlCopyMemory(&PopBattery->Status, &BattStatus, sizeof(BattStatus));
            break;
        }

        case POP_CB_QUERY_BATTERY_ESTIMATION_TIME_MODE:
        {
            /*
             * We have the estimated time remaining of the battery life.
             * Take away the processing mode request.
             */
            PopBattery->Flags &= ~POP_CB_PROCESSING_MODE_REQUEST;

            /* Copy the battery estimation time */
            EstTime = *(PULONG)Irp->AssociatedIrp.SystemBuffer;
            PopBattery->EstimatedBatteryTime = EstTime;

            /*
             * As we got fresh battery data at this point of processing this
             * request, alert whatever threads that waited on new battery data.
             */
            KeSetEvent(&PopFreshBatteryDataEvent, IO_NO_INCREMENT, FALSE);
            break;
        }

        default:
        {
            /*
             * We got an unknown battery request mode of which we have no idea
             * what to do. This is a serious bug in our end so behead the system.
             */
            KeBugCheckEx(INTERNAL_POWER_ERROR,
                         0x300,
                         POP_BATTERY_UNKNOWN_MODE_REQUEST,
                         (ULONG_PTR)DeviceObject,
                         (ULONG_PTR)Irp);
        }
    }

    /*
     * Cache the current executed operation mode so that the composite
     * battery handler understands what has been processed.
     */
    PopBattery->PreviousMode = Mode;

    /* Free the current IRP so that the handler can issue a new one */
    IoFreeIrp(PopBattery->Irp);
    PopBattery->Irp = NULL;

InvokeHandler:
    /* Allocate a policy workitem data so that we can invoke the CB handler */
    WorkItemData = PopAllocatePool(sizeof(POP_DEVICE_POLICY_WORKITEM_DATA),
                                   FALSE,
                                   TAG_PO_POLICY_DEVICE_WORKITEM_DATA);
    NT_ASSERT(WorkItemData != NULL);

    WorkItemData->PolicyData = NULL;
    WorkItemData->PolicyType = PolicyDeviceBattery;
    ExInitializeWorkItem(&WorkItemData->WorkItem,
                         PopCompositeBatteryHandler,
                         WorkItemData);

    /* Enqueue the control switch back to the handler */
    ExQueueWorkItem(&WorkItemData->WorkItem, DelayedWorkQueue);
    return STATUS_MORE_PROCESSING_REQUIRED;
}

static
ULONG
PopCalculateWaitStatusTimeout(VOID)
{
    PAGED_CODE();

    /*
     * If the event of new battery data is still in non-signaled state then
     * it means the threads that seek battery state information (typically
     * when you query such information with a call of NtPowerInformation) are
     * still waiting and the Power Manager has not provided fresh battery data yet.
     * In this scenario we want the Battery Class driver to return information as
     * soon as possible.
     */
    if (!KeReadStateEvent(&PopFreshBatteryDataEvent))
    {
        return 0;
    }

    return 0xFFFFFFFF;
}

static
VOID
PopCalculateBatteryCapacityThresholds(
    _Inout_ PBATTERY_WAIT_STATUS WaitStatus)
{
    ULONG EstimatedCapacity;
    ULONG FullChargeCapacity;

    PAGED_CODE();

    /*
     * Calculate the estimated current charge capacity based on the amount of
     * resolutions the system must be notified of battery capacity state change
     * and the full charge capacity of this battery.
     */
    FullChargeCapacity = PopBattery->BattInfo.FullChargedCapacity;
    EstimatedCapacity = (FullChargeCapacity * PopDefaultPowerPolicy->BroadcastCapacityResolution) / 100;

    /*
     * It could happen the full charge capacity is really low therefore we might
     * get an estimated capacity of 0, in this case we have to take the maximum
     * capacity (that is being 1).
     */
    EstimatedCapacity = max(1, EstimatedCapacity);

    /* Set the default low and high capacities, we will calculate them later */
    WaitStatus->LowCapacity = 0;
    WaitStatus->HighCapacity = 0xFFFFFFFF;

    /*
     * Assign the low capacity if we are sure the reported current capacity
     * from the battery status is above the estimated capacity.
     */
    if (PopBattery->Status.Capacity > EstimatedCapacity)
    {
        WaitStatus->LowCapacity = PopBattery->Status.Capacity - EstimatedCapacity;
    }

    /* Always set up the highest capacity, regardless if the current reported capacity is low */
    WaitStatus->HighCapacity = PopBattery->Status.Capacity + EstimatedCapacity;
}

/* PUBLIC FUNCTIONS ***********************************************************/

VOID
NTAPI
PopMarkNewBatteryPending(VOID)
{
    PopBattery->Flags |= POP_CB_PENDING_NEW_BATTERY;
}

VOID
NTAPI
PopQueryBatteryState(
    _Out_ PSYSTEM_BATTERY_STATE BatteryState)
{
    PAGED_CODE();

    /* The policy lock must be owned as when quering battery state */
    POP_ASSERT_POWER_POLICY_LOCK_OWNERSHIP();

    /* Fill the battery state field with zeroes */
    RtlZeroMemory(BatteryState, sizeof(*BatteryState));

    /*
     * No composite battery was ever connected with the Power Manager,
     * so directly indicate that.
     */
    if (PopBattery->Flags & POP_CB_NO_BATTERY)
    {
        BatteryState->AcOnLine = TRUE;
        BatteryState->BatteryPresent = FALSE;
        return;
    }

    /*
     * We must wait on the Power Manager to fetch for us the latest
     * battery state updates. We have to release the policy lock
     * while waiting so that the rest of the paging code can
     * continue execution.
     */
    PopReleasePowerPolicyLock();
    KeWaitForSingleObject(&PopFreshBatteryDataEvent, Executive, KernelMode, TRUE, NULL);
    PopAcquirePowerPolicyLock();

    /*
     * We got signaled from the Power Manager it has done fetching battery data.
     * However we must ensure the battery did not disappear while we were waiting.
     */
    if (PopBattery->Flags & POP_CB_NO_BATTERY)
    {
        /*
         * Something happened while we waited (an I/O error coming from the composite
         * battery driver) or the user physically removed the battery.
         */
        BatteryState->AcOnLine = TRUE;
        BatteryState->BatteryPresent = FALSE;
        KeClearEvent(&PopFreshBatteryDataEvent);
        return;
    }

    /*
     * Set the event to non-signaled state to avoid further callers thinking
     * the event was already signaled.
     */
    KeClearEvent(&PopFreshBatteryDataEvent);

    /*
     * If the system is powered by external AC source (power charging cord plugged in)
     * with battery fully charged, acknowledge that.
     */
    BatteryState->AcOnLine = FALSE;
    if (PopBattery->Status.PowerState & BATTERY_POWER_ON_LINE)
    {
        BatteryState->AcOnLine = TRUE;
    }

    /* At least one battery is present at this point */
    BatteryState->BatteryPresent = TRUE;

    /* If the battery is charging, acknowledge that */
    BatteryState->Charging = FALSE;
    if (PopBattery->Status.PowerState & BATTERY_CHARGING)
    {
        BatteryState->Charging = TRUE;
    }

    /* If the battery is discharging, acknowledge that */
    BatteryState->Discharging = FALSE;
    if (PopBattery->Status.PowerState & BATTERY_DISCHARGING)
    {
        BatteryState->Discharging = TRUE;
    }

    /* Return the rest of the battery state data to caller */
    BatteryState->MaxCapacity = PopBattery->BattInfo.FullChargedCapacity;
    BatteryState->RemainingCapacity = PopBattery->Status.Capacity;
    BatteryState->Rate = PopBattery->Status.Rate;
    BatteryState->EstimatedTime = PopBattery->EstimatedBatteryTime;
    BatteryState->DefaultAlert1 = PopBattery->BattInfo.DefaultAlert1;
    BatteryState->DefaultAlert2 = PopBattery->BattInfo.DefaultAlert2;
}

_Use_decl_annotations_
VOID
NTAPI
PopCompositeBatteryHandler(
    _In_ PVOID Parameter)
{
    PIRP Irp;
    ULONG Tag;
    PVOID IoBuffer;
    BATTERY_QUERY_INFORMATION QueryInfo;
    BATTERY_WAIT_STATUS WaitStatus;
    ULONG IoControlCode;
    PIO_STACK_LOCATION IrpStack;
    PDEVICE_OBJECT DeviceObject;
    ULONG InputBufferLength, OutputBufferLength;
    PPOP_DEVICE_POLICY_WORKITEM_DATA WorkItemData = (PPOP_DEVICE_POLICY_WORKITEM_DATA)Parameter;

    PAGED_CODE();

    /* We entered into a device policy handler, acquire the policy lock */
    PopAcquirePowerPolicyLock();

    /*
     * Ensure we got the right policy device as this handler only processes
     * composite batteries and not anything else. We do not care about about
     * the policy data as the kernel holds the composite battery data into
     * a global variable, PopBattery.
     */
    ASSERT(WorkItemData->PolicyType == PolicyDeviceBattery);

    /*
     * A battery is about to be removed. This means we got an error of which no
     * possible solution could be thought to handle it, or the battery just
     * simply disappeared.
     */
    if (PopBattery->Flags & POP_CB_REMOVE_BATTERY)
    {
        PopRemoveCompositeBattery();
        PopReleasePowerPolicyLock();
        PopFreePool(WorkItemData, TAG_PO_POLICY_DEVICE_WORKITEM_DATA);
        return;
    }

    /*
     * Ensure the composite battery does not have an outstanding IRP that still needs
     * to be completed. The CB IRP completion handler is responsible to free it once
     * it has got data from the ACPI driver of which the IRP is no longer needed.
     */
    ASSERT(PopBattery->Irp == NULL);

    /* Allocate a new fresh IRP to satisfy the required request */
    DeviceObject = PopBattery->DeviceObject;
    Irp = IoAllocateIrp(DeviceObject->StackSize, FALSE);
    if (!Irp)
    {
        /*
         * Failing this request on our end is fatal. Most likely the I/O manager
         * tried so hard to allocate an IRP for us it failed on doing so, due to
         * a serious low memory condition. Not something that we can do to salvage
         * the system so kill it.
         */
        KeBugCheckEx(INTERNAL_POWER_ERROR,
                     0,
                     POP_DEVICE_POLICY_IRP_ALLOC_FAILED,
                     PolicyDeviceBattery,
                     (ULONG_PTR)DeviceObject);
    }

    /*
     * We do not have any composite battery policy connected and we got a battery
     * for the first time, or we got a new one, or the I/O request has been cancelled
     * and we have to retry the operation back from the beginning. We must query the
     * tag information in order to gather battery information and other related data.
     */
    if ((PopBattery->Flags & POP_CB_NO_BATTERY) ||
        (PopBattery->Flags & POP_CB_PENDING_NEW_BATTERY) ||
        (PopBattery->Flags & POP_CB_RETRY_IO_REQUEST))
    {
        DPRINT1("Connect battery in progress, requesting battery tag (next mode -- POP_CB_READ_TAG_MODE)\n");

        /* Setup the necessary lengths for the battery information to query */
        InputBufferLength = sizeof(ULONG);
        OutputBufferLength = sizeof(ULONG);

        /* Setup the battery tag ourselves so that the battery driver can fill it in */
        Tag = 0;
        IoBuffer = &Tag;

        /* Setup the appropriate I/O control code and mode to query the tag */
        IoControlCode = IOCTL_BATTERY_QUERY_TAG;
        PopBattery->Mode = POP_CB_READ_TAG_MODE;

        /* Acknowledge the Power Manager the policy battery is processing a mode request */
        PopBattery->Flags |= POP_CB_PROCESSING_MODE_REQUEST;
    }
    else
    {
        /*
         * We already have a connected composite battery with a tag, we must read
         * the battery information in order to proceed with battery status and such.
         * For this we must check the prior mode request and act accordingly on that mode.
         */
        if (PopBattery->PreviousMode == POP_CB_READ_TAG_MODE)
        {
            DPRINT1("Querying battery information in progress (next mode -- POP_CB_QUERY_INFORMATION_MODE)\n");

            /* The battery should have already processed this request */
            POP_ASSERT_NO_BATTERY_REQUEST_MODE();

            /* We have a tag, setup the appropriate lengths to query battery information */
            InputBufferLength = sizeof(BATTERY_QUERY_INFORMATION);
            OutputBufferLength = sizeof(BATTERY_INFORMATION);

            /*
             * By this point we should already have a battery tag that identifies this
             * battery, otherwise we got bogus data from the Battery Class driver.
             */
            ASSERT(PopBattery->BatteryTag != 0);

            /* Setup the right query battery information for this operation */
            RtlZeroMemory(&QueryInfo, sizeof(QueryInfo));
            QueryInfo.BatteryTag = PopBattery->BatteryTag;
            QueryInfo.InformationLevel = BatteryInformation;
            IoBuffer = &QueryInfo;

            /* Setup the appropriate I/O control code and mode to query the tag */
            IoControlCode = IOCTL_BATTERY_QUERY_INFORMATION;
            PopBattery->Mode = POP_CB_QUERY_INFORMATION_MODE;

            /* Acknowledge the Power Manager the policy battery is processing a mode request */
            PopBattery->Flags |= POP_CB_PROCESSING_MODE_REQUEST;
        }
        else if (PopBattery->PreviousMode == POP_CB_QUERY_INFORMATION_MODE)
        {
            /*
             * We have gotten the required battery information in order
             * to read battery status.
             */
            DPRINT1("Querying battery status in progress (next mode -- POP_CB_QUERY_STATUS_MODE)\n");

            /* The battery should have already processed this request */
            POP_ASSERT_NO_BATTERY_REQUEST_MODE();

            /* Setup the appropriate lengths to query battery status */
            InputBufferLength = sizeof(BATTERY_WAIT_STATUS);
            OutputBufferLength = sizeof(BATTERY_STATUS);

            /*
             * Fill in battery tag and the information required to read the
             * status of the battery.
             */
            RtlZeroMemory(&WaitStatus, sizeof(WaitStatus));
            WaitStatus.BatteryTag = PopBattery->BatteryTag;
            WaitStatus.Timeout = PopCalculateWaitStatusTimeout();
            WaitStatus.PowerState = PopBattery->Status.PowerState;

            /* Calculate the low and high capacities of which we should be notified of new battery status */
            PopCalculateBatteryCapacityThresholds(&WaitStatus);

            /* Assign the I/O buffer to that of the battery wait status */
            IoBuffer = &WaitStatus;

            /* Setup the appropriate I/O control code and mode to query the tag */
            IoControlCode = IOCTL_BATTERY_QUERY_STATUS;
            PopBattery->Mode = POP_CB_QUERY_INFORMATION_MODE;

            /* Acknowledge the Power Manager the policy battery is processing a mode request */
            PopBattery->Flags |= POP_CB_PROCESSING_MODE_REQUEST;
        }
        else if (PopBattery->PreviousMode == POP_CB_QUERY_STATUS_MODE)
        {
            /*
             * We have successfully read the battery status, the last operation
             * is to query the estimated time of the battery after a successful
             * read of the battery status.
             */
            DPRINT1("Querying battery estimation time in progress (next mode -- POP_CB_QUERY_BATTERY_ESTIMATION_TIME_MODE)\n");

            /* The battery should have already processed this request */
            POP_ASSERT_NO_BATTERY_REQUEST_MODE();

            /* Setup the appropriate lengths to query the battery estimation time */
            InputBufferLength = sizeof(BATTERY_QUERY_INFORMATION);
            OutputBufferLength = sizeof(ULONG);

            /* Setup the right query battery information for this operation */
            RtlZeroMemory(&QueryInfo, sizeof(QueryInfo));
            QueryInfo.BatteryTag = PopBattery->BatteryTag;
            QueryInfo.InformationLevel = BatteryEstimatedTime;
            QueryInfo.AtRate = 0;
            IoBuffer = &QueryInfo;

            /* Setup the appropriate I/O control code and mode to query the tag */
            IoControlCode = IOCTL_BATTERY_QUERY_INFORMATION;
            PopBattery->Mode = POP_CB_QUERY_BATTERY_ESTIMATION_TIME_MODE;

            /* Acknowledge the Power Manager the policy battery is processing a mode request */
            PopBattery->Flags |= POP_CB_PROCESSING_MODE_REQUEST;
        }
        else if (PopBattery->PreviousMode == POP_CB_QUERY_BATTERY_ESTIMATION_TIME_MODE)
        {
            /*
             * We have finally queried the battery estimation time, now we
             * have to repeat the operation by reading the battery status again.
             */
            DPRINT1("Battery estimation time queried, rolling back to read battery status (next mode -- POP_CB_QUERY_STATUS_MODE)\n");

            /* The battery should have already processed this request */
            POP_ASSERT_NO_BATTERY_REQUEST_MODE();

            /* Setup the appropriate lengths to query battery status */
            InputBufferLength = sizeof(BATTERY_WAIT_STATUS);
            OutputBufferLength = sizeof(BATTERY_STATUS);

            /*
             * Fill in battery tag and the information required to read the
             * status of the battery.
             */
            RtlZeroMemory(&WaitStatus, sizeof(WaitStatus));
            WaitStatus.BatteryTag = PopBattery->BatteryTag;
            WaitStatus.Timeout = PopCalculateWaitStatusTimeout();
            WaitStatus.PowerState = PopBattery->Status.PowerState;

            /* Calculate the low and high capacities of which we should be notified of new battery status */
            PopCalculateBatteryCapacityThresholds(&WaitStatus);

            /* Assign the I/O buffer to that of the battery wait status */
            IoBuffer = &WaitStatus;

            /* Setup the appropriate I/O control code and mode to query the tag */
            IoControlCode = IOCTL_BATTERY_QUERY_STATUS;
            PopBattery->Mode = POP_CB_QUERY_INFORMATION_MODE;

            /* Acknowledge the Power Manager the policy battery is processing a mode request */
            PopBattery->Flags |= POP_CB_PROCESSING_MODE_REQUEST;
        }
        else
        {
            /*
             * We received an unknown operation mode request of which we have no
             * idea on what to do. This is a bug in our end, so punt the system.
             */
            KeBugCheckEx(INTERNAL_POWER_ERROR,
                         0x301,
                         POP_BATTERY_UNKNOWN_MODE_REQUEST,
                         (ULONG_PTR)DeviceObject,
                         (ULONG_PTR)Irp);
        }
    }

    /* Setup the IRP stack parameters based on the requested operation */
    IrpStack = IoGetNextIrpStackLocation(Irp);
    IrpStack->MajorFunction = IRP_MJ_DEVICE_CONTROL;
    IrpStack->Parameters.DeviceIoControl.IoControlCode = IoControlCode;
    IrpStack->Parameters.DeviceIoControl.InputBufferLength = InputBufferLength;
    IrpStack->Parameters.DeviceIoControl.OutputBufferLength = OutputBufferLength;

    /* Setup the system buffer based on the requested operation */
    Irp->AssociatedIrp.SystemBuffer = IoBuffer;

    /*
     * Register the IRP completion CB handler and give the IRP to the composite battery,
     * so that when the Battery Class driver returns back our IRP the completion handler
     * will further process the CB operation event.
     */
    PopBattery->Irp = Irp;
    IoSetCompletionRoutine(Irp,
                           PopBatteryIrpCompletion,
                           NULL,
                           TRUE,
                           TRUE,
                           TRUE);

    /*
     * We are going to leave the device policy handler soon, release the lock
     * before dispatching the IRP to the Battery Class driver.
     */
    PopReleasePowerPolicyLock();
    IoCallDriver(DeviceObject, Irp);

    /* Free the work item data as we no longer need it */
    PopFreePool(WorkItemData, TAG_PO_POLICY_DEVICE_WORKITEM_DATA);
}

NTSTATUS
NTAPI
PopRemoveCompositeBattery(VOID)
{
    PIRP Irp;
    PDEVICE_OBJECT DeviceObject;

    PAGED_CODE();

    DPRINT1("Removing battery %wZ (Device 0x%p)\n", PopBattery->BatteryName, PopBattery->DeviceObject);

    /* The policy lock must be owned as we remove the composite battery */
    POP_ASSERT_POWER_POLICY_LOCK_OWNERSHIP();

    /* Enforce the AC power policy as the battery disappeared */
    if (PopDefaultPowerPolicy != &PopAcPowerPolicy)
    {
        PopSetDefaultPowerPolicy(PolicyAc);

        /* FIXME: We must recalculate the thermal throttle gauges and system idleness here */
    }

    /* Discard the battery device driver name */
    RtlInitUnicodeString(&PopBattery->BatteryName, NULL);

    /* Report to the system that we do not have system batteries globally */
    PopCapabilities.SystemBatteriesPresent = FALSE;

    /* Clear any of the flags that have been set before and enforce the "no battery" flag */
    PopBattery->Flags &= ~(POP_CB_PENDING_NEW_BATTERY | POP_CB_PROCESSING_MODE_REQUEST | POP_CB_RETRY_IO_REQUEST | POP_CB_REMOVE_BATTERY);
    PopBattery->Flags |= POP_CB_NO_BATTERY;

    /*
     * Alert any threads that were waiting on for latest updated battery
     * status, only to find out for them that the battery disappeared.
     */
    KeSetEvent(&PopFreshBatteryDataEvent, IO_NO_INCREMENT, FALSE);

    /* Free the IRP and take the reference onto the battery device object we held away */
    Irp = PopBattery->Irp;
    IoFreeIrp(Irp);
    PopBattery->Irp = NULL;

    DeviceObject = PopBattery->DeviceObject;
    ObDereferenceObject(DeviceObject);
    PopBattery->DeviceObject = NULL;

    /* Reset the battery operation modes */
    PopBattery->Mode = POP_CB_NO_MODE;
    PopBattery->PreviousMode = POP_CB_NO_MODE;

    /* And finally reset all the battery information and status data */
    PopBattery->BatteryTag = 0;
    PopBattery->Temperature = 0;
    RtlZeroMemory(&PopBattery->Status, sizeof(PopBattery->Status));
    RtlZeroMemory(&PopBattery->BattInfo, sizeof(PopBattery->BattInfo));
    PopBattery->EstimatedBatteryTime = 0;
    return STATUS_SUCCESS;
}

NTSTATUS
NTAPI
PopAddCompositeBattery(
    _In_ PDEVICE_OBJECT BatteryDevice,
    _In_ PUNICODE_STRING BatteryName)
{
    PAGED_CODE();

    DPRINT1("Adding a battery from %wZ (Device 0x%p)\n", BatteryName, BatteryDevice);

    /* Is this the first time that a battery gets added? */
    if (!PopCapabilities.SystemBatteriesPresent)
    {
        /*
         * Report that system batteries are present to the system globally
         * if we have not done it before.
         */
        PopCapabilities.SystemBatteriesPresent = TRUE;

        /* Copy the battery name into the kernel CB structure */
        RtlCopyUnicodeString(&PopBattery->BatteryName, BatteryName);

        /*
         * The system must have been running on AC power at this point of
         * receiving a system battery. Switch the power policy to that of
         * DC policy.
         */
        ASSERT(PopDefaultPowerPolicy == &PopAcPowerPolicy);
        PopSetDefaultPowerPolicy(PolicyDc);
    }

    /*
     * Associate the battery device with the composite battery (CB) kernel
     * structure. The device policy manager will kick the respective battery
     * policy handler to read the battery tag for the first time.
     */
    PopBattery->DeviceObject = BatteryDevice;
    return STATUS_SUCCESS;
}

/* EOF */
