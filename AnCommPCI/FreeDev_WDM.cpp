#include "FreeDev_WDM.h"
#include <initguid.h>
#include "guid.h"
#include "Ioctls.h"

/*  Ntoskrnl.lib函数库相关函数声明*/
extern "C" NTKERNELAPI PHYSICAL_ADDRESS MmGetPhysicalAddress (IN PVOID BaseAddress);
extern "C" PVOID   MmAllocateContiguousMemory( __in SIZE_T  NumberOfBytes, __in PHYSICAL_ADDRESS  HighestAcceptableAddress);

/************************************************************************
* 函数名称:DriverEntry
* 功能描述:初始化驱动程序，定位和申请硬件资源，创建内核对象
* 参数列表:
      pDriverObject:从I/O管理器中传进来的驱动对象
      pRegistryPath:驱动程序在注册表的中的路径
* 返回 值:返回初始化驱动状态
*************************************************************************/
#pragma INITCODE 
extern "C" NTSTATUS DriverEntry(IN PDRIVER_OBJECT pDriverObject,
								IN PUNICODE_STRING pRegistryPath)
{
	KdPrint(("Enter DriverEntry\n"));

	pDriverObject->DriverExtension->AddDevice = WDMAddDevice;
	pDriverObject->MajorFunction[IRP_MJ_PNP] = WDMPnp;
	pDriverObject->MajorFunction[IRP_MJ_CLEANUP] = WDMCleanupRoutine;
	pDriverObject->MajorFunction[IRP_MJ_DEVICE_CONTROL] = DeviceIOControl;
	pDriverObject->MajorFunction[IRP_MJ_CREATE] = WDMDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_CLOSE] = WDMDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_READ] = WDMDispatchRoutine;
	pDriverObject->MajorFunction[IRP_MJ_WRITE] = WDMDispatchRoutine;
	pDriverObject->DriverUnload = WDMUnloadDevice;

	KdPrint(("Leave DriverEntry\n"));
	return STATUS_SUCCESS;
}

/************************************************************************
* 函数名称:WDMAddDevice
* 功能描述:添加新设备
* 参数列表:
      DriverObject:从I/O管理器中传进来的驱动对象
      PhysicalDeviceObject:从I/O管理器中传进来的物理设备对象
* 返回 值:返回添加新设备状态
*************************************************************************/
#pragma PAGEDCODE
NTSTATUS WDMAddDevice(IN PDRIVER_OBJECT DriverObject,
                           IN PDEVICE_OBJECT PhysicalDeviceObject)
{ 
	PAGED_CODE();
	KdPrint(("Enter WDMAddDevice\n"));

	NTSTATUS status;
	PDEVICE_OBJECT fdo;
	PDEVICE_OBJECT pDevice;
	
	status = IoCreateDevice(
		DriverObject,
		sizeof(DEVICE_EXTENSION),
		NULL,//没有指定设备名
		FILE_DEVICE_UNKNOWN,
		0,
		FALSE,
		&fdo);
	if( !NT_SUCCESS(status))
		return status;
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)fdo->DeviceExtension;
	pdx->fdo = fdo;
	pdx->pdo = PhysicalDeviceObject;
	pdx->NextStackDevice = IoAttachDeviceToDeviceStack(fdo, PhysicalDeviceObject);
	
	//创建设备接口
	status = IoRegisterDeviceInterface(PhysicalDeviceObject, &MY_WDM_DEVICE, NULL, &pdx->interfaceName);
	if( !NT_SUCCESS(status))
	{
		IoDeleteDevice(fdo);
		return status;
	}
	KdPrint(("%wZ\n",&pdx->interfaceName));
	IoSetDeviceInterfaceState(&pdx->interfaceName, TRUE);

	if( !NT_SUCCESS(status))
	{
		if( !NT_SUCCESS(status))
		{
			return status;
		}
	}

	fdo->Flags |= DO_BUFFERED_IO | DO_POWER_PAGABLE;
	fdo->Flags &= ~DO_DEVICE_INITIALIZING;

	KdPrint(("Leave WDMAddDevice\n"));
	return STATUS_SUCCESS;
}

/************************************************************************
* 函数名称:DefaultPnpHandler
* 功能描述:对PNP IRP进行缺省处理
* 参数列表:
      pdx:设备对象的扩展
      Irp:从IO请求包
* 返回 值:返回状态
*************************************************************************/ 
#pragma PAGEDCODE
NTSTATUS DefaultPnpHandler(PDEVICE_EXTENSION pdx, PIRP Irp)
{
	PAGED_CODE();
	KdPrint(("Enter DefaultPnpHandler\n"));
	IoSkipCurrentIrpStackLocation(Irp);
	KdPrint(("Leave DefaultPnpHandler\n"));
	return IoCallDriver(pdx->NextStackDevice, Irp);
}

#pragma LOCKEDCODE
NTSTATUS OnRequestComplete(PDEVICE_OBJECT junk, PIRP Irp, PKEVENT pev)
{							// OnRequestComplete
	//在完成例程中设置等待事件
	KeSetEvent(pev, 0, FALSE);
	//标志本IRP还需要再次被完成
	return STATUS_MORE_PROCESSING_REQUIRED;
}

///////////////////////////////////////////////////////////////////////////////

#pragma PAGEDCODE
NTSTATUS ForwardAndWait(PDEVICE_EXTENSION pdx, PIRP Irp)
{							// ForwardAndWait
	PAGED_CODE();
	
	KEVENT event;
	//初始化事件
	KeInitializeEvent(&event, NotificationEvent, FALSE);

	//将本层堆栈拷贝到下一层堆栈
	IoCopyCurrentIrpStackLocationToNext(Irp);
	//设置完成例程
	IoSetCompletionRoutine(Irp, (PIO_COMPLETION_ROUTINE) OnRequestComplete,
		(PVOID) &event, TRUE, TRUE, TRUE);

	//调用底层驱动，即PDO
	IoCallDriver(pdx->NextStackDevice, Irp);
	//等待PDO完成
	KeWaitForSingleObject(&event, Executive, KernelMode, FALSE, NULL);
	return Irp->IoStatus.Status;
}							// ForwardAndWait


/************************************************************************
* 函数名称:HandleRemoveDevice
* 功能描述:对IRP_MN_REMOVE_DEVICE IRP进行处理
* 参数列表:
      fdo:功能设备对象
      Irp:从IO请求包
* 返回 值:返回状态
*************************************************************************/
#pragma PAGEDCODE
NTSTATUS HandleRemoveDevice(PDEVICE_EXTENSION pdx, PIRP Irp)
{
	PAGED_CODE();
	KdPrint(("Enter HandleRemoveDevice\n"));
	

	//IoDisconnectInterrupt(pdx->pInterruptObject);

	if (pdx->MemOK == 1)
	{
		MmFreeContiguousMemory(pdx->pReadDMAData);
		MmFreeContiguousMemory(pdx->pWriteDMAData);
		MmFreeContiguousMemory(pdx->pReadDMATable);
		MmFreeContiguousMemory(pdx->pWriteDMATable);
	}



	//断开内存映射
	if(pdx->nMem>=1)MmUnmapIoSpace(pdx->MemBar0,pdx->nMem0);
	if(pdx->nMem>=2)MmUnmapIoSpace(pdx->MemBar1,pdx->nMem1);
	if(pdx->nMem>=3)MmUnmapIoSpace(pdx->MemBar2,pdx->nMem2);
	
  Irp->IoStatus.Status = STATUS_SUCCESS;
	NTSTATUS status = DefaultPnpHandler(pdx, Irp);

	IoSetDeviceInterfaceState(&pdx->interfaceName, FALSE);
	RtlFreeUnicodeString(&pdx->interfaceName);

	//调用IoDetachDevice()把fdo从设备栈中脱开：
	if (pdx->NextStackDevice)
		IoDetachDevice(pdx->NextStackDevice);
	
	//删除fdo：
	IoDeleteDevice(pdx->fdo);
    
	return status;
}

#pragma PAGEDCODE
NTSTATUS InitMyPCI(IN PDEVICE_EXTENSION pdx,IN PCM_PARTIAL_RESOURCE_LIST list)
{
	PDEVICE_OBJECT fdo = pdx->fdo;

	ULONG vector;
	KIRQL irql;
	KINTERRUPT_MODE mode;
	KAFFINITY affinity;
	BOOLEAN irqshare;
	BOOLEAN gotinterrupt = FALSE;

	PHYSICAL_ADDRESS portbase;
	BOOLEAN gotport = FALSE;

	NTSTATUS status;



	PCM_PARTIAL_RESOURCE_DESCRIPTOR resource = &list->PartialDescriptors[0];
	ULONG nres = list->Count;
	pdx->nMem=0;
	for (ULONG i = 0; i < nres; ++i, ++resource)
		{						// for each resource
		switch (resource->Type)
			{					// switch on resource type
		case CmResourceTypePort:
			portbase = resource->u.Port.Start;
			pdx->nports = resource->u.Port.Length;
			pdx->mappedport = (resource->Flags & CM_RESOURCE_PORT_IO) == 0;
			gotport = TRUE;
			break;

		case CmResourceTypeMemory:
			if (pdx->nMem==0) 
			{
				pdx->MemBar0 = (PUCHAR)MmMapIoSpace(resource->u.Memory.Start,
					resource->u.Memory.Length,
					MmNonCached);
				pdx->nMem0 = resource->u.Memory.Length;
				pdx->nMem++;
			}else if(pdx->nMem==1)
			{
				pdx->MemBar1 = (PUCHAR)MmMapIoSpace(resource->u.Memory.Start,
					resource->u.Memory.Length,
					MmNonCached);
				pdx->nMem1 = resource->u.Memory.Length;
				pdx->nMem++;
			}else if(pdx->nMem==2)
			{
				pdx->MemBar2 = (PUCHAR)MmMapIoSpace(resource->u.Memory.Start,
					resource->u.Memory.Length,
					MmNonCached);
				pdx->nMem2 = resource->u.Memory.Length;
				pdx->nMem++;
			}

			break;

		case CmResourceTypeInterrupt:


			IO_CONNECT_INTERRUPT_PARAMETERS params;

			// deviceExtension is a pointer to the driver's device extension. 
			//     deviceExtension->IntObj is a PKINTERRUPT.
			// deviceInterruptService is a pointer to the driver's InterruptService routine.
			// IntResource is a CM_PARTIAL_RESOURCE_DESCRIPTOR structure of either type CmResourceTypeInterrupt or CmResourceTypeMessageInterrupt.
			// PhysicalDeviceObject is a pointer to the device's PDO. 
			// ServiceContext is a pointer to driver-specified context for the ISR.

			RtlZeroMemory(&params, sizeof(IO_CONNECT_INTERRUPT_PARAMETERS));
			params.Version = CONNECT_FULLY_SPECIFIED;
			params.FullySpecified.PhysicalDeviceObject = pdx->pdo;
			params.FullySpecified.InterruptObject = &pdx->pInterruptObject;
			params.FullySpecified.ServiceRoutine = (PKSERVICE_ROUTINE)ISRInterrupt;
			params.FullySpecified.ServiceContext = (PVOID)pdx;
			params.FullySpecified.FloatingSave = FALSE;
			params.FullySpecified.SpinLock = NULL;

			if (resource->Flags & CM_RESOURCE_INTERRUPT_MESSAGE) {
				// The resource is for a message-signaled interrupt. Use the u.MessageInterrupt.Translated member of IntResource.
				KdPrint(("Init MSI Interrupt "));
				params.FullySpecified.Vector = resource->u.MessageInterrupt.Translated.Vector;
				params.FullySpecified.Irql = (KIRQL)resource->u.MessageInterrupt.Translated.Level;
				params.FullySpecified.SynchronizeIrql = (KIRQL)resource->u.MessageInterrupt.Translated.Level;
				params.FullySpecified.ProcessorEnableMask = resource->u.MessageInterrupt.Translated.Affinity;
			}
			else {
				// The resource is for a line-based interrupt. Use the u.Interrupt member of IntResource.
				KdPrint(("Init line-based Interrupt "));
				params.FullySpecified.Vector = resource->u.Interrupt.Vector;
				params.FullySpecified.Irql = (KIRQL)resource->u.Interrupt.Level;
				params.FullySpecified.SynchronizeIrql = (KIRQL)resource->u.Interrupt.Level;
				params.FullySpecified.ProcessorEnableMask = resource->u.Interrupt.Affinity;
			}

			params.FullySpecified.InterruptMode = (resource->Flags & CM_RESOURCE_INTERRUPT_LATCHED ? Latched : LevelSensitive);
			params.FullySpecified.ShareVector = (BOOLEAN)(resource->ShareDisposition == CmResourceShareShared);

	//		status = IoConnectInterruptEx(&params);

	

			//////
			//if (resource->Flags & CM_RESOURCE_INTERRUPT_MESSAGE)
			//{
			//	KdPrint(("Interrupt MSI!\n"));
			//}
			//else
			//{
			//	KdPrint(("Interrupt Normal!\n"));
			//	
			//}

			//irql = (KIRQL)resource->u.Interrupt.Level;
			//vector = resource->u.Interrupt.Vector;
			//affinity = resource->u.Interrupt.Affinity;
			//mode = (resource->Flags & CM_RESOURCE_INTERRUPT_LATCHED)
			//	? Latched : LevelSensitive;
			//irqshare = resource->ShareDisposition == CmResourceShareShared;

			//gotinterrupt = TRUE;
			//
			////KeInitializeDpc(&pdx->fdo->Dpc, DPCForISR, NULL);
			//status=IoConnectInterrupt(&pdx->pInterruptObject, (PKSERVICE_ROUTINE)ISRInterrupt,
			//	(PVOID)pdx, NULL, vector, irql, irql, LevelSensitive, irqshare, affinity, FALSE);

			/*if (!NT_SUCCESS(status))
			{
				KdPrint(("Error!\n"));
				
			}
			else
			{
				KdPrint(("OK!\n"));
			}
			*/


			break;

		default:
			KdPrint(("Unexpected I/O resource type %d\n", resource->Type));
			break;
			}					// switch on resource type
		}						// for each resource

	

	
	if (pdx->mappedport)
		{						// map port address for RISC platform
		pdx->portbase = (PUCHAR) MmMapIoSpace(portbase, pdx->nports, MmNonCached);
		if (!pdx->mappedport)
			{
			KdPrint(("Unable to map port range %I64X, length %X\n", portbase, pdx->nports));
			return STATUS_INSUFFICIENT_RESOURCES;
			}
		}						// map port address for RISC platform
	else
		pdx->portbase = (PUCHAR) portbase.QuadPart;



		
	// Init DMA Memory



	pdx->pReadDMA = (DMA_Operation*)(((unsigned char *)pdx->MemBar2) + 0);
	pdx->pWriteDMA = (DMA_Operation*)(((unsigned char *)pdx->MemBar2) + 16);

	
	return STATUS_SUCCESS;	
}

#pragma PAGEDCODE
NTSTATUS HandleStartDevice(PDEVICE_EXTENSION pdx, PIRP Irp)
{
	PAGED_CODE();
	KdPrint(("Enter HandleStartDevice\n"));

	//转发IRP并等待返回
	NTSTATUS status = ForwardAndWait(pdx,Irp);
	if (!NT_SUCCESS(status))
	{
		Irp->IoStatus.Status = status;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
		return status;
	}

	//得到当前堆栈
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);

	//从当前堆栈得到翻译信息
	PCM_PARTIAL_RESOURCE_LIST translated;
	if (stack->Parameters.StartDevice.AllocatedResourcesTranslated)
		translated = &stack->Parameters.StartDevice.AllocatedResourcesTranslated->List[0].PartialResourceList;
	else
		translated = NULL;

	KdPrint(("Init the PCI card!\n"));
	status=InitMyPCI(pdx,translated);

	
	if (!NT_SUCCESS(status))
	{
		KdPrint(("Init the PCI card ERROR !\n"));	
		Irp->IoStatus.Status = status;
		IoCompleteRequest(Irp, IO_NO_INCREMENT);
		return status;	
	}
	//完成IRP
	Irp->IoStatus.Status = STATUS_SUCCESS;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);

	KdPrint(("Leave HandleStartDevice\n"));
	return status;
}

/************************************************************************
* 函数名称:WDMPnp
* 功能描述:对即插即用IRP进行处理
* 参数列表:
      fdo:功能设备对象
      Irp:从IO请求包
* 返回 值:返回状态
*************************************************************************/
#pragma PAGEDCODE
NTSTATUS WDMPnp(IN PDEVICE_OBJECT fdo,
                        IN PIRP Irp)
{
	PAGED_CODE();
		
	KdPrint(("Enter WDMPnp\n"));
	NTSTATUS status = STATUS_SUCCESS;
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) fdo->DeviceExtension;
	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	static NTSTATUS (*fcntab[])(PDEVICE_EXTENSION pdx, PIRP Irp) = 
	{
		HandleStartDevice,		// IRP_MN_START_DEVICE
		DefaultPnpHandler,		// IRP_MN_QUERY_REMOVE_DEVICE
		HandleRemoveDevice,		// IRP_MN_REMOVE_DEVICE
		DefaultPnpHandler,		// IRP_MN_CANCEL_REMOVE_DEVICE
		DefaultPnpHandler,		// IRP_MN_STOP_DEVICE
		DefaultPnpHandler,		// IRP_MN_QUERY_STOP_DEVICE
		DefaultPnpHandler,		// IRP_MN_CANCEL_STOP_DEVICE
		DefaultPnpHandler,		// IRP_MN_QUERY_DEVICE_RELATIONS
		DefaultPnpHandler,		// IRP_MN_QUERY_INTERFACE
		DefaultPnpHandler,		// IRP_MN_QUERY_CAPABILITIES
		DefaultPnpHandler,		// IRP_MN_QUERY_RESOURCES
		DefaultPnpHandler,		// IRP_MN_QUERY_RESOURCE_REQUIREMENTS
		DefaultPnpHandler,		// IRP_MN_QUERY_DEVICE_TEXT
		DefaultPnpHandler,		// IRP_MN_FILTER_RESOURCE_REQUIREMENTS
		DefaultPnpHandler,		// 
		DefaultPnpHandler,		// IRP_MN_READ_CONFIG
		DefaultPnpHandler,		// IRP_MN_WRITE_CONFIG
		DefaultPnpHandler,		// IRP_MN_EJECT
		DefaultPnpHandler,		// IRP_MN_SET_LOCK
		DefaultPnpHandler,		// IRP_MN_QUERY_ID
		DefaultPnpHandler,		// IRP_MN_QUERY_PNP_DEVICE_STATE
		DefaultPnpHandler,		// IRP_MN_QUERY_BUS_INFORMATION
		DefaultPnpHandler,		// IRP_MN_DEVICE_USAGE_NOTIFICATION
		DefaultPnpHandler,		// IRP_MN_SURPRISE_REMOVAL
	};

	ULONG fcn = stack->MinorFunction;
	if (fcn >= arraysize(fcntab))
	{						// 未知的子功能代码
		status = DefaultPnpHandler(pdx, Irp); // some function we don't know about
		return status;
	}						

#if DBG
	static char* fcnname[] = 
	{
		"IRP_MN_START_DEVICE",
		"IRP_MN_QUERY_REMOVE_DEVICE",
		"IRP_MN_REMOVE_DEVICE",
		"IRP_MN_CANCEL_REMOVE_DEVICE",
		"IRP_MN_STOP_DEVICE",
		"IRP_MN_QUERY_STOP_DEVICE",
		"IRP_MN_CANCEL_STOP_DEVICE",
		"IRP_MN_QUERY_DEVICE_RELATIONS",
		"IRP_MN_QUERY_INTERFACE",
		"IRP_MN_QUERY_CAPABILITIES",
		"IRP_MN_QUERY_RESOURCES",
		"IRP_MN_QUERY_RESOURCE_REQUIREMENTS",
		"IRP_MN_QUERY_DEVICE_TEXT",
		"IRP_MN_FILTER_RESOURCE_REQUIREMENTS",
		"",
		"IRP_MN_READ_CONFIG",
		"IRP_MN_WRITE_CONFIG",
		"IRP_MN_EJECT",
		"IRP_MN_SET_LOCK",
		"IRP_MN_QUERY_ID",
		"IRP_MN_QUERY_PNP_DEVICE_STATE",
		"IRP_MN_QUERY_BUS_INFORMATION",
		"IRP_MN_DEVICE_USAGE_NOTIFICATION",
		"IRP_MN_SURPRISE_REMOVAL",
	};

	KdPrint(("PNP Request (%s)\n", fcnname[fcn]));
#endif // DBG

	status = (*fcntab[fcn])(pdx, Irp);
	KdPrint(("Leave WDMPnp\n"));
	return status;
}

/************************************************************************
* 函数名称:WDMDispatchRoutine
* 功能描述:对缺省IRP进行处理
* 参数列表:
      fdo:功能设备对象
      Irp:从IO请求包
* 返回 值:返回状态
*************************************************************************/
#pragma PAGEDCODE
NTSTATUS WDMDispatchRoutine(IN PDEVICE_OBJECT fdo,
								 IN PIRP Irp)
{
	PAGED_CODE();
	KdPrint(("Enter WDMDispatchRoutine\n"));
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;	// no bytes xfered
	IoCompleteRequest( Irp, IO_NO_INCREMENT );
	KdPrint(("Leave WDMDispatchRoutine\n"));
	return STATUS_SUCCESS;
}

/************************************************************************
* 函数名称:WDMCleanupRoutine
* 功能描述:对IRP_MJ_CLEANUP IRP进行处理
* 参数列表:
      fdo:功能设备对象
      Irp:从IO请求包
* 返回 值:返回状态
*************************************************************************/
#pragma PAGEDCODE
NTSTATUS WDMCleanupRoutine(IN PDEVICE_OBJECT fdo,
								 IN PIRP Irp)
{
	PAGED_CODE();
	KdPrint(("Enter WDMCleanupRoutine\n"));
	
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)fdo->DeviceExtension;
	
	Irp->IoStatus.Status = STATUS_SUCCESS;
	Irp->IoStatus.Information = 0;	// no bytes xfered
	IoCompleteRequest( Irp, IO_NO_INCREMENT );
	KdPrint(("Leave WDMCleanupRoutine\n"));
	return STATUS_SUCCESS;
}


/************************************************************************
* 函数名称:WDMUnloadDevice
* 功能描述:负责驱动程序的卸载操作
* 参数列表:
      DriverObject:驱动对象
* 返回 值:返回状态
*************************************************************************/
#pragma PAGEDCODE
void WDMUnloadDevice(IN PDRIVER_OBJECT DriverObject)
{
	PAGED_CODE();
	KdPrint(("Enter WDMUnloadDevice\n"));
	KdPrint(("Leave WDMUnloadDevice\n"));
}

NTSTATUS CompleteRequest(IN PIRP Irp, IN NTSTATUS status, IN ULONG_PTR info)
{							// CompleteRequest
	Irp->IoStatus.Status = status;
	Irp->IoStatus.Information = info;
	IoCompleteRequest(Irp, IO_NO_INCREMENT);
	return status;
}


// 2011.03.22 ljs add
#pragma PAGEDCODE


NTSTATUS DeviceIOControl(PDEVICE_OBJECT fdo, PIRP Irp)
{							// DispatchControl
	PAGED_CODE();
	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION) fdo->DeviceExtension;
	
	NTSTATUS status;

	//KdPrint(("Enter DeviceIOControl\n"));
	ULONG info = 0;
	

	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
	ULONG cbin = stack->Parameters.DeviceIoControl.InputBufferLength;
	ULONG cbout = stack->Parameters.DeviceIoControl.OutputBufferLength;
	ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;
	PCHAR bar_addr;
	ULONG reg = 0x00000000; 
	unsigned long temp;
			

		switch (code)
		{	
			case IOCTL_ALLOC_MEM:
			{
				KdPrint(("Driver Version 0.9.0.24\n"));
				KdPrint(("IOCTL_ALLOC_MEM\n"));
				if (pdx->MemOK == 0)
				{
					PHYSICAL_ADDRESS maxAddress;

					maxAddress.u.LowPart = 0xFFFFFFFF;
					maxAddress.u.HighPart = 0;

					// DMA Read Table
					pdx->pReadDMATable = (DMA_Struct *)MmAllocateContiguousMemory(sizeof(DMA_Struct), maxAddress);
					pdx->pReadDMATablePhyAddr = (unsigned char *)MmGetPhysicalAddress(pdx->pReadDMATable).LowPart;

					// DMA Write Table
					pdx->pWriteDMATable = (DMA_Struct *)MmAllocateContiguousMemory(sizeof(DMA_Struct), maxAddress);
					pdx->pWriteDMATablePhyAddr = (unsigned char *)MmGetPhysicalAddress(pdx->pWriteDMATable).LowPart;


					
					pdx->pWriteDMAData = (unsigned char *)MmAllocateContiguousMemory(0x8000 * 4, maxAddress);
					pdx->pWriteDMADataPhyAddr = (unsigned char *)MmGetPhysicalAddress(pdx->pWriteDMAData).LowPart;


					// DMA Read Data  
					pdx->pReadDMAData = (unsigned char *)MmAllocateContiguousMemory(0x10000 * 4, maxAddress);
					pdx->pReadDMADataPhyAddr = (unsigned char *)MmGetPhysicalAddress(pdx->pReadDMAData).LowPart;
					
				
					pdx->MemOK = 1;
				}
				status = STATUS_SUCCESS;
				break;
			}
			case IOCTL_SEND_DATA:
			{
				KdPrint(("IOCTL_SEND_DATA\n"));
				DMASendToDevice(pdx, (unsigned char *)Irp->AssociatedIrp.SystemBuffer, cbin);
				status = STATUS_SUCCESS;
				break;
			}
			case IOCTL_BEGIN_RECEIVE_DATA:
			{
			//	KdPrint(("IOCTL_BEGIN_RECEIVE_DATA:%d\n", ((ReadCmd *)Irp->AssociatedIrp.SystemBuffer)->length));
				
				if (pdx->ReadDMATableOnline == 0)
				{
					pdx->ReadOK = 0;
					pdx->ReadDMATableOnline = 1;
					WRITE_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0xa0),1);

					if (!pdx->notFirstData)
					{
						
						pdx->currentReadOffset = READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0xa8));
						KdPrint(("B,Device let me read:%d\n",pdx->currentReadOffset));
						pdx->notFirstData = 1;
					}

				//	DMAReceiveFromDevice(pdx, pdx->currentReadOffset, ((ReadCmd *)Irp->AssociatedIrp.SystemBuffer)->length);

			//		KdPrint(("DMA Online."));
				}
				status = STATUS_SUCCESS;
				break;
			}
			case IOCTL_RECEIVE_DATA:
			{
				ISRInterrupt(NULL, pdx);
				//KdPrint(("IOCTL_RECEIVE_DATA\n"));
				if (pdx->ReadOK)
				{
					
		

					if (pdx->readDMADataLen > cbout)
						pdx->readDMADataLen = cbout;
				
					
					memcpy((unsigned char *)Irp->AssociatedIrp.SystemBuffer, pdx->pReadDMAData, pdx->readDMADataLen);


					pdx->ReadDMATableOnline = 0;
					status = STATUS_SUCCESS;
					info = pdx->readDMADataLen;
					KdPrint(("ReadOK Len:%d\n", info));

				}
				else
				{
					//KdPrint(("ReadNoOK\n"));
					status = STATUS_SUCCESS;
					info = 0;
				}
				break;
			}
			case IOCTL_RECEIVE_IGNORE_LEN_DATA:
			{
				ISRInterrupt(NULL, pdx);
			//	KdPrint(("IOCTL_RECEIVE_DATA\n"));
				if (pdx->ReadOK)
				{
					unsigned long len = 0;
					len = cbout;


					memcpy((unsigned char *)Irp->AssociatedIrp.SystemBuffer, pdx->pReadDMAData, len);

					
					pdx->ReadDMATableOnline = 0;
					status = STATUS_SUCCESS;
					info = len;
			//		KdPrint(("ReadOK\n"));
			//		KdPrint(("INFO:%d\n", info));

				}
				else
				{
					//KdPrint(("ReadNoOK\n"));
					status = STATUS_SUCCESS;
					info = 0;
				}
				break;
			}
			case IOCTL_GET_DEVICE_STATUS:
			{
				DeviceStatus deviceStatus;

				memcpy(((unsigned char *)(&deviceStatus)) + 4, ((unsigned char *)(pdx->MemBar2)) + 0x50, 20);
				deviceStatus.comm = (READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x40))) & 0x00000001;
				deviceStatus.commOperation = (READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x48))) & 0x00000001;
				unsigned long copyLen = (sizeof(DeviceStatus) > cbout ? cbout : sizeof(DeviceStatus));
				memcpy((unsigned char *)Irp->AssociatedIrp.SystemBuffer, &deviceStatus, copyLen);
				status = STATUS_SUCCESS;
				info = copyLen;
				
				break;
			}	
			case IOCTL_RESET_COMM:
			{				
						unsigned long temp=	READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x48));
						temp |= 0x00000001;
						WRITE_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x48), temp);

				pdx->ReadOK = 0;
				pdx->ReadDMATableOnline = 0;
				pdx->notFirstData = 0;
				status = STATUS_SUCCESS;
				info = 0;

				break;
			}
			case IOCTL_RESET_COMM_2:
			{

				unsigned long temp = READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x48));
				temp &= 0xFFFFFFFE;
				WRITE_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x48), temp);			

				pdx->ReadOK = 0;
				pdx->ReadDMATableOnline = 0;
				pdx->notFirstData = 0;
				status = STATUS_SUCCESS;
				info = 0;

				break;
			}
			case IOCTL_CHECK_RECEIVE_FINISHED:
			{
				//KdPrint(("BEGIN CHECK FINISHED"));
				unsigned long temp = READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0xA4)) & 0x00000001;
				
				
			

				unsigned long maybeFinished = 0;
				temp |= READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x80)) & 0x00000001;
			
				temp |= READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x90)) & 0x00000001;
			
				if (temp == 0)
				{
					KdPrint(("0xA4 is %d\n", READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0xA4))));
					KdPrint(("0x80 is %d\n", READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x80))));
					KdPrint(("0x90 is %d\n", READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x90))));


					KdPrint(("I think it is finished."));
					maybeFinished = 1;

					pdx->ReadOK = 0;
					pdx->ReadDMATableOnline = 0;
					pdx->notFirstData = 0;
				}
				else
				{
		//			KdPrint(("some data is not readed."));
					maybeFinished = 0;
				}
				memcpy((unsigned char *)Irp->AssociatedIrp.SystemBuffer, (unsigned char *)&maybeFinished, sizeof(unsigned long));

			//	KdPrint(("END CHECK FINISHED"));
				status = STATUS_SUCCESS;
				info = sizeof(unsigned long);

				break;
			}

			default:
				status = STATUS_INVALID_DEVICE_REQUEST;
				break;

		}					
		//KdPrint(("Leave DeviceIOControl\n"));
		return CompleteRequest(Irp, status, info);


}



void DMASendToDevice(IN PDEVICE_EXTENSION pdx, IN unsigned char * memAddr, IN unsigned long length)
{

	memset(pdx->pWriteDMAData, 0, length);
	memcpy(pdx->pWriteDMAData, memAddr, length);
	if (length % 4)
	{
		length = length / 4 + 1;
	}
	else
	{
		length = length / 4;
	}

	memset((unsigned char *)pdx->pWriteDMATable, 0, sizeof(DMA_Struct));
	pdx->pWriteDMATable->DMA_Length0 = length;
	pdx->pWriteDMATable->AddrOnBoard0 = 0;
	pdx->pWriteDMATable->AddrOnMemHi0 = 0;
	pdx->pWriteDMATable->AddrOnMemLo0 = (unsigned long)pdx->pWriteDMADataPhyAddr;

	pdx->pWriteDMA->Setting = 0x0000ffff; // Clear  
	pdx->pWriteDMA->Setting |= (0 << 17) + 0;  //Set MSI Enable
	pdx->pWriteDMA->TablePhyAddrHi = 0;
	pdx->pWriteDMA->TablePhyAddrLo = (unsigned long)pdx->pWriteDMATablePhyAddr;
	pdx->pWriteDMA->DoOperation = 0;


}

void DMAReceiveFromDevice(IN PDEVICE_EXTENSION pdx, IN unsigned long offset, IN unsigned long length)
{
//	memset(pdx->pReadDMAData, 0, length);
	if (length % 4)
	{
		length = length / 4 + 1;
	}
	else
	{
		length = length / 4;
	}
	unsigned long len0 = (length > 0x8000 ? 0x8000 : length);
	unsigned long len1 = (length > 0x8000 ? length - 0x8000 : 0);

	unsigned long sendItems = 0;
	if (len0 > 0)
		sendItems++;
	if (len1 > 0)
		sendItems++;

	memset((unsigned char *)pdx->pReadDMATable, 0, sizeof(DMA_Struct));

	if (sendItems >= 1)
	{
		pdx->pReadDMATable->DMA_Length0 = len0;
		pdx->pReadDMATable->AddrOnBoard0 = offset;
		pdx->pReadDMATable->AddrOnMemHi0 = 0;
		pdx->pReadDMATable->AddrOnMemLo0 = (unsigned long)pdx->pReadDMADataPhyAddr;
	}
	if (sendItems >= 2)
	{
		pdx->pReadDMATable->DMA_Length1 = len1;
		pdx->pReadDMATable->AddrOnBoard1 = offset + 0x8000 * 4;
		pdx->pReadDMATable->AddrOnMemHi1 = 0;
		pdx->pReadDMATable->AddrOnMemLo1 = (unsigned long)(pdx->pReadDMADataPhyAddr + 0x8000 * 4);
	}
	pdx->pReadDMA->Setting = 0x0000ffff; // Clear  
	pdx->pReadDMA->Setting |= (1 << 17) + (1 << 18) + (sendItems-1);  //Set MSI Enable
	pdx->pReadDMA->TablePhyAddrHi = 0;
	pdx->pReadDMA->TablePhyAddrLo = (unsigned long)pdx->pReadDMATablePhyAddr;
	pdx->pReadDMA->DoOperation = (sendItems - 1);
	pdx->intCount = sendItems;

}


BOOLEAN ISRInterrupt(PKINTERRUPT InterruptObject, PDEVICE_EXTENSION pdx)
{		
/*
	KdPrint(("Enter ISRInterrupt\n"));

	KdPrint(("Current 0xa8 is %d.\n", READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0xa8)));
	KdPrint(("Current keeped a8 is %d.\n", pdx->currentReadOffset));


	KdPrint(("Current 0x80 is %d.\n", READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x80)));
	KdPrint(("Current 0x88 is %d.\n", READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x88)));
	KdPrint(("Current 0x90 is %d.\n", READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x90)));
	KdPrint(("Current 0x98 is %d.\n", READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x98)));
	*/
	//pdx->intCount--;
	//if (pdx->intCount)
	//	goto intExit;// TRUE;
	//KeInsertQueueDpc(&pdx->fdo->Dpc, pdx->fdo, pdx->fdo->CurrentIrp);

	unsigned long len = 0;
	if (pdx->currentReadOffset == 0x00000000)
	{
		if (!(READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x80)) & 0x00000002))
			goto intExit;
		KdPrint(("0x00000000 ReadOK."));
		len = (READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x84))) * 4;

	}
	else if (pdx->currentReadOffset == 0x00040000)
	{
		if (!(READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x90)) & 0x00000002))
			goto intExit;
		KdPrint(("0x00040000 ReadOK."));
		len = (READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x94))) * 4;
	}

	pdx->readDMADataLen = len;
	//READ_REGISTER_BUFFER_UCHAR(((unsigned char *)pdx->MemBar0) + pdx->currentReadOffset, pdx->pReadDMAData, pdx->readDMADataLen);
	memcpy(pdx->pReadDMAData, ((unsigned char *)pdx->MemBar0) + pdx->currentReadOffset, pdx->readDMADataLen);
	KdPrint(("Current pkg's first data is %d.\n", *(((unsigned char *)pdx->MemBar0) + pdx->currentReadOffset+8)));

	unsigned long tempoffset = pdx->currentReadOffset;
	
	//pdx->currentReadOffset = READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0xa8);
	//KdPrint(("R,Device let me read:%d\n", pdx->currentReadOffset));
	if (tempoffset == 0x00000000)
	{ 
		pdx->currentReadOffset = 0x00040000;
		KdPrint(("R,Device let me read:%d\n", pdx->currentReadOffset));
		KdPrint(("Write 0x88 to 1 by temp value:%d\n",tempoffset));
		do
		{
			WRITE_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x88), 1);
			unsigned long temp80 = READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x80));
			KdPrint(("Current 0x80 is:%d\n", temp80));
			if (temp80 & 0x00000002)
			{
				break;
			}

			if (!(temp80 & 0x00000020))
			{				
				break;
			}

		} while (1);
		

	//	KdPrint(("Current 0xa8 is %d.\n", READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0xa8)));
		//KdPrint(("Current keeped a8 is %d.\n", pdx->currentReadOffset));

	//	KdPrint(("Current 0x80 is %d.\n", READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x80)));
	//	KdPrint(("Current 0x88 is %d.\n", READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x88)));
	//	KdPrint(("Current 0x90 is %d.\n", READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x90)));
	//	KdPrint(("Current 0x98 is %d.\n", READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x98)));
	}
	else if (tempoffset == 0x00040000)
	{
		pdx->currentReadOffset = 0x00000000;
		KdPrint(("R,Device let me read:%d\n", pdx->currentReadOffset));
		KdPrint(("Write 0x98 to 1 by temp value:%d\n", tempoffset));
		do
		{

			WRITE_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x98) , 1);

			unsigned long temp90 = READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x80));
			KdPrint(("Current 0x90 is:%d\n", temp90));
			if (temp90 & 0x00000002)
			{
				break;
			}

			if (!(temp90 & 0x00000020))
			{
				break;
			}

		} while (1);
		
	//	KdPrint(("Current 0xa8 is %d.\n", READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0xa8)));
	//	KdPrint(("Current keeped a8 is %d.\n", pdx->currentReadOffset));

	//	KdPrint(("Current 0x80 is %d.\n", READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x80)));
	//	KdPrint(("Current 0x88 is %d.\n", READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x88)));
	//	KdPrint(("Current 0x90 is %d.\n", READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x90)));
	//	KdPrint(("Current 0x98 is %d.\n", READ_REGISTER_ULONG((unsigned long *)(((unsigned char *)pdx->MemBar2) + 0x98)));
	}


	pdx->ReadOK = 1;
	
	
	
	intExit:
	//KdPrint(("Exit ISRInterrupt\n"));
	return TRUE;

}
//
//VOID DPCForISR(IN PKDPC Dpc, IN PVOID Context, IN  PVOID fdo, IN PVOID pIrp)
//{
//	KdPrint(("Enter DPCForISR\n"));
//	PDEVICE_EXTENSION pdx = (PDEVICE_EXTENSION)((PDEVICE_OBJECT)fdo)->DeviceExtension;
//
//	if (pdx->pLastIrp == 0)
//	{
//		KdPrint(("pdx->pLastIrp == 0\n"));
//		KdPrint(("Exit DPCForISR\n"));
//		return;
//	}
//	PIRP Irp = pdx->pLastIrp;
//	pdx->pLastIrp = 0;
//	PIO_STACK_LOCATION stack = IoGetCurrentIrpStackLocation(Irp);
//	ULONG cbin = stack->Parameters.DeviceIoControl.InputBufferLength;
//	ULONG cbout = stack->Parameters.DeviceIoControl.OutputBufferLength;
//	ULONG code = stack->Parameters.DeviceIoControl.IoControlCode;
//	ULONG info = 0;
//	unsigned long temp = 0;
//	NTSTATUS status;
//	switch (code)
//	{	// process request
//
//		case IOCTL_SEND_DATA:
//		{
//			MmFreeContiguousMemory(pdx->pWriteDMAData);
//			KdPrint(("IOCTL_SEND_DATA\n"));
//			status = STATUS_SUCCESS;			
//			break;
//		}
//		case IOCTL_RECEIVE_DATA:
//		{
//			KdPrint(("IOCTL_RECEIVE_DATA\n"));
//			status = STATUS_SUCCESS;
//			if (cbout <= pdx->lReceivedLength + pdx->lLastReceivedLength)
//			{
//				memcpy(((unsigned char *)Irp->AssociatedIrp.SystemBuffer + pdx->lReceivedLength), pdx->pReadDMAData, pdx->lLastReceivedLength);
//				KdPrint(("IOCTL_RECEIVE_DATA FILL: %d %d\n",pdx->lReceivedLength ,pdx->lLastReceivedLength));
//			}
//			else
//				KdPrint(("!!!!!DANGER!!!!\n"));
//			MmFreeContiguousMemory(pdx->pReadDMAData);
//			pdx->lReceivedLength += pdx->lLastReceivedLength;
//
//			if (pdx->lReceivedLength < cbout)
//			{
//				temp = cbout - pdx->lReceivedLength;
//				if (temp > MAX_RECEIVE_LENGTH)
//					temp = MAX_RECEIVE_LENGTH;
//				DMAReceiveFromDevice(pdx, pdx->lReceivedLength, temp);
//				return;
//			}
//
//			info = pdx->lReceivedLength;
//			pdx->lReceivedLength = 0;
//			pdx->lLastReceivedLength = 0;
//			status = STATUS_SUCCESS;
//			break;
//		}
//	}
//	CompleteRequest(Irp, status, info);
//	KdPrint(("Exit DPCForISR\n"));
//}
