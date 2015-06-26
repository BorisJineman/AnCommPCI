/************************************************************************
* 文件名称:HelloWDM.h                                                 
* 作    者:张帆
* 完成日期:2007-11-1
*************************************************************************/

#ifdef __cplusplus
extern "C"
{
#endif
//#include <NTDDk.h>
#include <wdm.h>
#ifdef __cplusplus
}
#endif 


typedef struct _DMA_Struct
{
	unsigned long HEAD[4];

	unsigned long DMA_Length0;
	unsigned long AddrOnBoard0;
	unsigned long AddrOnMemHi0;
	unsigned long AddrOnMemLo0;

	unsigned long DMA_Length1;
	unsigned long AddrOnBoard1;
	unsigned long AddrOnMemHi1;
	unsigned long AddrOnMemLo1;

	unsigned long EMPTY[4];
} DMA_Struct, *PDMA_Struct;


typedef struct _DMA_Operation
{
	unsigned long Setting;
	unsigned long TablePhyAddrHi;
	unsigned long TablePhyAddrLo;
	unsigned long DoOperation;
}DMA_Operation, *PDMA_Operation;

typedef struct _DEVICE_EXTENSION
{
	PDEVICE_OBJECT fdo;
	PDEVICE_OBJECT pdo;
	PDEVICE_OBJECT NextStackDevice;
	UNICODE_STRING interfaceName;			

	PUCHAR portbase;									
	ULONG nports;											
	ULONG nMem;												
	PVOID MemBar0;							
	ULONG nMem0;							
	PVOID MemBar1;							
	ULONG nMem1;							
	PVOID MemBar2;							
	ULONG nMem2;							
	BOOLEAN mappedport;						
	PUCHAR pConfigSpace;		
		
	unsigned long MemOK;

	unsigned long ReadDMATableOnline;

	unsigned long currentReadOffset;
	unsigned long notFirstData;
	unsigned long ReadOK;


	PKINTERRUPT pInterruptObject;

	unsigned char * pReadDMATablePhyAddr;
	unsigned char * pWriteDMATablePhyAddr;
	unsigned char * pReadDMADataPhyAddr;
	unsigned char * pWriteDMADataPhyAddr;

	DMA_Struct * pReadDMATable;
	DMA_Struct * pWriteDMATable;
	unsigned char * pReadDMAData;
	unsigned long readDMADataLen;
	unsigned char * pWriteDMAData;

	DMA_Operation * pReadDMA;
	DMA_Operation * pWriteDMA;



} DEVICE_EXTENSION, *PDEVICE_EXTENSION;



#define PAGEDCODE code_seg("PAGE")
#define LOCKEDCODE code_seg()
#define INITCODE code_seg("INIT")

#define PAGEDDATA data_seg("PAGE")
#define LOCKEDDATA data_seg()
#define INITDATA data_seg("INIT")

#define arraysize(p) (sizeof(p)/sizeof((p)[0]))

NTSTATUS WDMAddDevice(IN PDRIVER_OBJECT DriverObject,
                           IN PDEVICE_OBJECT PhysicalDeviceObject);
NTSTATUS WDMPnp(IN PDEVICE_OBJECT fdo,
                        IN PIRP Irp);
NTSTATUS WDMDispatchRoutine(IN PDEVICE_OBJECT fdo,
								 IN PIRP Irp);
NTSTATUS WDMCleanupRoutine(IN PDEVICE_OBJECT fdo,
								 IN PIRP Irp);								 
void WDMUnloadDevice(IN PDRIVER_OBJECT DriverObject);

NTSTATUS DeviceIOControl(PDEVICE_OBJECT fdo, PIRP Irp);


extern "C"
NTSTATUS DriverEntry(IN PDRIVER_OBJECT DriverObject,
                     IN PUNICODE_STRING RegistryPath);

void DMASendToDevice(IN PDEVICE_EXTENSION pdx, IN unsigned char * memAddr, IN unsigned long length);
void DMAReceiveFromDevice(IN PDEVICE_EXTENSION pdx, IN unsigned long offset, IN unsigned long length);

BOOLEAN ISRInterrupt(PKINTERRUPT InterruptObject, PDEVICE_EXTENSION pdx);

VOID DPCForISR(IN PKDPC Dpc, IN PVOID Context, IN  PVOID fdo, IN PVOID pIrp);