#ifndef CXBKERNALEXPORTS_H
#define CXBKERNALEXPORTS_H

#include "..\utils\Stdafx.h"

EXTERN_C CONST CHAR* ExLoadedImageName;
CONST CHAR* GetLoadedImageName();

typedef unsigned __int64		u64;
typedef unsigned char           u8;
typedef unsigned short          u16;
typedef unsigned int            u32;

typedef signed char             s8;
typedef signed short            s16;
typedef signed int              s32;
typedef signed __int64          s64;

typedef unsigned __int64		QWORD;

typedef long NTSTATUS;

#define STATUS_SUCCESS	0
#define NT_EXTRACT_ST(Status)	((((ULONG)(Status)) >> 30)& 0x3)
#define NT_SUCCESS(Status)		(((NTSTATUS)(Status)) >= 0)
#define NT_INFORMATION(Status)	(NT_EXTRACT_ST(Status) == 1)
#define NT_WARNING(Status)		(NT_EXTRACT_ST(Status) == 2)
#define NT_ERROR(Status)		(NT_EXTRACT_ST(Status) == 3)

// Valid values for the Attributes field
#define OBJ_INHERIT             0x00000002L
#define OBJ_PERMANENT           0x00000010L
#define OBJ_EXCLUSIVE           0x00000020L
#define OBJ_CASE_INSENSITIVE    0x00000040L
#define OBJ_OPENIF              0x00000080L
#define OBJ_OPENLINK            0x00000100L
#define OBJ_VALID_ATTRIBUTES    0x000001F2L

typedef struct _STRING
{
	USHORT Length;
	USHORT MaximumLength;
	PCHAR Buffer;
} STRING, *PSTRING;

typedef struct _OBJECT_ATTRIBUTES
{
    HANDLE RootDirectory;
    PSTRING ObjectName;
    ULONG Attributes;
} OBJECT_ATTRIBUTES, *POBJECT_ATTRIBUTES;

typedef struct _IO_STATUS_BLOCK
{
	union
	{
        NTSTATUS Status;
        PVOID Pointer;
    } st;
    ULONG_PTR Information;
} IO_STATUS_BLOCK, *PIO_STATUS_BLOCK;

typedef VOID (NTAPI *PIO_APC_ROUTINE)(
    IN PVOID ApcContext,
    IN PIO_STATUS_BLOCK IoStatusBlock,
    IN ULONG Reserved
);

typedef enum _TEMP_INDEX
{
    CPU = 0,
    GPU,
    MEM,
    BRD
} TEMP_INDEX;

extern "C" // Calls to SMC message function in xboxkrnl.lib
{
	VOID HalReturnToFirmware(DWORD mode);
	void __stdcall HalSendSMCMessage(void* input, void* output);
	DWORD XeKeysGetKey(DWORD keyNum, VOID* buffer, DWORD* bufferSize);
	void RtlInitAnsiString(PSTRING DestinationString, const char*  SourceString);
	// Xex exports
	UINT32 __stdcall NtSetSystemTime(LPFILETIME, LPFILETIME);

	VOID XeCryptSha(BYTE * pbInp1, DWORD cbInp1, 
		BYTE * pbInp2, DWORD cbInp2, 
		BYTE * pbInp3, DWORD cbInp3, 
		BYTE * pbOut, DWORD cbOut);

	NTSTATUS
	NTAPI
	NtOpenFile(
		OUT		PHANDLE FileHandle,
		IN		ACCESS_MASK DesiredAccess,
		IN		POBJECT_ATTRIBUTES ObjectAttributes,
		OUT		PIO_STATUS_BLOCK IoStatusBlock,
		IN		ULONG ShareAccess,
		IN		ULONG OpenOptions );

	NTSTATUS
	NTAPI
	NtDeviceIoControlFile(
		IN		HANDLE FileHandle,
		IN		HANDLE Event OPTIONAL,
		IN		PIO_APC_ROUTINE ApcRoutine OPTIONAL,
		IN		PVOID ApcContext OPTIONAL,
		OUT		PIO_STATUS_BLOCK IoStatusBlock,
		IN		ULONG IoControlCode,
		IN		PVOID InputBuffer OPTIONAL,
		IN		ULONG InputBufferLength,
		OUT		PVOID OutputBuffer OPTIONAL,
		IN		ULONG OutputBufferLength
		);

	NTSTATUS
	NTAPI
	NtClose(
		IN		HANDLE Handle
		);

	int ObCreateSymbolicLink(STRING*, STRING*);
	int ObDeleteSymbolicLink(STRING*);
}

class CXBKernalExports
{
public:
	static void RebootXbox();
	static void ShutdownXbox();
	static int QueryTemps(TEMP_INDEX sTempIndex);
};

#endif //CXBKERNALEXPORTS_H