#include "DDKCommon.h"
#include "MyMemoryIo64.h"
#include "HwidHook.h"
#include <ntddndis.h>

VOID NtDeviceIoControlFileCallback(ULONG64 IoControlCode, ULONG64 InputBuffer, ULONG64 InputBufferLength, ULONG64 OutputBuffer, ULONG64 OutputBufferLength) {
	//��ʱirql == 2 !
	// 
	//�޸�����Mac��ַ����
	if (IoControlCode == IOCTL_NDIS_QUERY_GLOBAL_STATS &&
		InputBufferLength >= 4 && MmiGetPhysicalAddress((PVOID)InputBuffer) && MmiGetPhysicalAddress((PVOID)(InputBuffer + 4 - 1)) &&
		OutputBufferLength >= 6 && MmiGetPhysicalAddress((PVOID)OutputBuffer) && MmiGetPhysicalAddress((PVOID)(OutputBuffer + 6 - 1))) {
		DWORD Code = *(DWORD*)(InputBuffer);
		switch (Code) {
			case OID_802_3_PERMANENT_ADDRESS:
			case OID_802_3_CURRENT_ADDRESS:
			case OID_802_5_PERMANENT_ADDRESS:
			case OID_802_5_CURRENT_ADDRESS:
			{
				PUCHAR pMac = (PUCHAR)OutputBuffer;
				pMac[0] = 0x00; pMac[1] = 0x11; pMac[2] = 0x22; pMac[3] = 0x33; pMac[4] = 0x44; pMac[5] = 0x55;
				break;
			}
			default:
				break;
		}
	}
}
VOID NtQueryVolumeInformationFileCallback(ULONG64 FsInformationClass, ULONG64 FsInformation, ULONG64 Length) {
	//��ʱirql == 2 !
	// 
	//�޸ķ������к�����
	switch (FsInformationClass)
	{
	case FileFsVolumeInformation:
	{
		if (Length >= sizeof(FILE_FS_VOLUME_INFORMATION) && 
			MmiGetPhysicalAddress((PVOID)FsInformation) && 
			MmiGetPhysicalAddress((PVOID)(FsInformation + sizeof(FILE_FS_VOLUME_INFORMATION) - 1))) {

			PFILE_FS_VOLUME_INFORMATION pinfo = (PFILE_FS_VOLUME_INFORMATION)FsInformation;
			pinfo->VolumeSerialNumber = 0;
		}
		break;
	}
	default:
		break;
	}
}
extern "C" NTSTATUS DriverEntry(PDRIVER_OBJECT drv, PUNICODE_STRING reg_path) {
	Mmi_Init();
	GetRealTime();//��ʼ��GetRealTime

	//�����Ƿ����� NtQueryVolumeInformationFile Hook,TRUEΪ����,FALSEΪ�ر�
	//ע��,win10 1507 - win10 1709��֧��NtQueryVolumeInformationFile Hook,��Ϊ�޷��Ӷ�ջ�л�ȡ������
	//NtQueryVolumeInformationFile Hook ��������win7�Լ�win10 1803�����ϰ汾
	setntqhookstats(FALSE);

	//����NtDeviceIoControlFile Hook��Callback,win7,win10ȫϵͳ����
	setdicprecabk(NtDeviceIoControlFileCallback);

	//����NtQueryVolumeInformationFile Hook��Callback
	setntqcabk(NtQueryVolumeInformationFileCallback);
	return STATUS_SUCCESS;
}
