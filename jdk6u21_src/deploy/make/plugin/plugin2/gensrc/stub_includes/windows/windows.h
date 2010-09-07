/* Windows #defines and typedefs required for processing of Windows APIs */

#define FAR
#define IN
#define OUT
#define WINBASEAPI
#define WINUSERAPI
#define WINAPI
#define APIENTRY
#define CONST const
#define VOID void
typedef int                 BOOL;
typedef int                 BOOL_PARAM;
typedef unsigned char       BYTE;
typedef char                CHAR;
typedef unsigned int        DWORD;
typedef unsigned int*       LPDWORD;
typedef int                 INT;
typedef int                 INT32;
typedef __int64             INT64;
typedef float               FLOAT;
typedef struct _handle*     HANDLE;
typedef HANDLE              HMODULE;
typedef HANDLE              HWND;
typedef long                LONG;
typedef long                SIZE_T;
typedef const char*         LPCSTR;
typedef void*               LPVOID;
typedef const void*         LPCVOID;
typedef struct _proc*       PROC;
typedef unsigned int*       PUINT;
typedef unsigned int        UINT;
typedef unsigned short      USHORT;
typedef unsigned short      WORD;

typedef struct _SECURITY_ATTRIBUTES {
    DWORD nLength;
    LPVOID lpSecurityDescriptor;
    BOOL bInheritHandle;
} SECURITY_ATTRIBUTES, *PSECURITY_ATTRIBUTES, *LPSECURITY_ATTRIBUTES;

typedef struct _OVERLAPPED {
} OVERLAPPED, *LPOVERLAPPED;

#define ERROR_PIPE_CONNECTED             535

/* #defines related to event and file mapping APIs below */
#define STATUS_ABANDONED_WAIT_0          0x00000080
#define WAIT_ABANDONED                   STATUS_ABANDONED_WAIT_0
#define STATUS_WAIT_0                    0x00000000
#define WAIT_OBJECT_0                    STATUS_WAIT_0
#define WAIT_TIMEOUT                     258
#define INFINITE                         0xFFFFFFFF  // Infinite timeout
// FIXME: GlueGen doesn't handle the -1 syntax, and won't emit 0xFFFFFFFFFFFFFFFF
// See the CustomJavaCode declaration in windows.cfg
#define INVALID_HANDLE_VALUE             -1

#define STANDARD_RIGHTS_REQUIRED         (0x000F0000L)
#define SYNCHRONIZE                      (0x00100000L)

// FIXME: GlueGen isn't yet capable of dealing with this complex #define
#define SECTION_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SECTION_QUERY|\
                            SECTION_MAP_WRITE |      \
                            SECTION_MAP_READ |       \
                            SECTION_MAP_EXECUTE |    \
                            SECTION_EXTEND_SIZE)

// FIXME: GlueGen isn't yet capable of dealing with this complex #define
//#define EVENT_ALL_ACCESS (STANDARD_RIGHTS_REQUIRED|SYNCHRONIZE|0x3)
#define EVENT_ALL_ACCESS 0x001F0003

//
// Define the dwOpenMode values for CreateNamedPipe
//

#define PIPE_ACCESS_INBOUND         0x00000001
#define PIPE_ACCESS_OUTBOUND        0x00000002
#define PIPE_ACCESS_DUPLEX          0x00000003

//
// Define the dwPipeMode values for CreateNamedPipe
//

#define PIPE_WAIT                   0x00000000
#define PIPE_NOWAIT                 0x00000001
#define PIPE_READMODE_BYTE          0x00000000
#define PIPE_READMODE_MESSAGE       0x00000002
#define PIPE_TYPE_BYTE              0x00000000
#define PIPE_TYPE_MESSAGE           0x00000004

//
// Define the well known values for CreateNamedPipe nMaxInstances
//

#define PIPE_UNLIMITED_INSTANCES    255

#define CREATE_NEW          1
#define CREATE_ALWAYS       2
#define OPEN_EXISTING       3
#define OPEN_ALWAYS         4
#define TRUNCATE_EXISTING   5

//
// Define access rights to files and directories
//

#define FILE_SHARE_READ                 0x00000001  
#define FILE_SHARE_WRITE                0x00000002  
#define FILE_SHARE_DELETE               0x00000004  
#define FILE_ATTRIBUTE_READONLY             0x00000001  
#define FILE_ATTRIBUTE_HIDDEN               0x00000002  
#define FILE_ATTRIBUTE_SYSTEM               0x00000004  
#define FILE_ATTRIBUTE_DIRECTORY            0x00000010  
#define FILE_ATTRIBUTE_ARCHIVE              0x00000020  
#define FILE_ATTRIBUTE_DEVICE               0x00000040  
#define FILE_ATTRIBUTE_NORMAL               0x00000080  
#define FILE_ATTRIBUTE_TEMPORARY            0x00000100  
#define FILE_ATTRIBUTE_SPARSE_FILE          0x00000200  
#define FILE_ATTRIBUTE_REPARSE_POINT        0x00000400  
#define FILE_ATTRIBUTE_COMPRESSED           0x00000800  
#define FILE_ATTRIBUTE_OFFLINE              0x00001000  
#define FILE_ATTRIBUTE_NOT_CONTENT_INDEXED  0x00002000  
#define FILE_ATTRIBUTE_ENCRYPTED            0x00004000  

//
// File creation flags must start at the high end since they
// are combined with the attributes
//

#define FILE_FLAG_WRITE_THROUGH         0x80000000
#define FILE_FLAG_OVERLAPPED            0x40000000
#define FILE_FLAG_NO_BUFFERING          0x20000000
#define FILE_FLAG_RANDOM_ACCESS         0x10000000
#define FILE_FLAG_SEQUENTIAL_SCAN       0x08000000
#define FILE_FLAG_DELETE_ON_CLOSE       0x04000000
#define FILE_FLAG_BACKUP_SEMANTICS      0x02000000
#define FILE_FLAG_POSIX_SEMANTICS       0x01000000
#define FILE_FLAG_OPEN_REPARSE_POINT    0x00200000
#define FILE_FLAG_OPEN_NO_RECALL        0x00100000
#define FILE_FLAG_FIRST_PIPE_INSTANCE   0x00080000

//
//  These are the generic rights.
//

#define GENERIC_READ                     0x80000000
#define GENERIC_WRITE                    0x40000000
#define GENERIC_EXECUTE                  0x20000000
#define GENERIC_ALL                      0x10000000

/* Windows API declarations */

WINBASEAPI
BOOL_PARAM
WINAPI
CloseHandle(
    IN OUT HANDLE hObject
    );

WINBASEAPI
HANDLE
WINAPI
CreateEventA(
    IN LPSECURITY_ATTRIBUTES lpEventAttributes,
    IN BOOL_PARAM bManualReset,
    IN BOOL_PARAM bInitialState,
    IN LPCSTR lpName
    );

WINBASEAPI
HANDLE
WINAPI
OpenEventA(
    IN DWORD dwDesiredAccess,
    IN BOOL_PARAM bInheritHandle,
    IN LPCSTR lpName
    );

WINBASEAPI
BOOL_PARAM
WINAPI
SetEvent(
    IN HANDLE hEvent
    );

WINBASEAPI
BOOL_PARAM
WINAPI
ResetEvent(
    IN HANDLE hEvent
    );

WINBASEAPI
DWORD
WINAPI
WaitForSingleObject(
    IN HANDLE hHandle,
    IN DWORD dwMilliseconds
    );

WINBASEAPI
DWORD
WINAPI
GetCurrentProcessId(
    VOID
    );

WINBASEAPI
HANDLE
WINAPI
CreateFileA(
    IN LPCSTR lpFileName,
    IN DWORD dwDesiredAccess,
    IN DWORD dwShareMode,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    IN DWORD dwCreationDisposition,
    IN DWORD dwFlagsAndAttributes,
    IN HANDLE hTemplateFile
    );

WINBASEAPI
HANDLE
WINAPI
CreateNamedPipeA(
    IN LPCSTR lpName,
    IN DWORD dwOpenMode,
    IN DWORD dwPipeMode,
    IN DWORD nMaxInstances,
    IN DWORD nOutBufferSize,
    IN DWORD nInBufferSize,
    IN DWORD nDefaultTimeOut,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes
    );

WINBASEAPI
BOOL_PARAM
WINAPI
ConnectNamedPipe(
    IN HANDLE hNamedPipe,
    IN LPOVERLAPPED lpOverlapped
    );

WINBASEAPI
BOOL_PARAM
WINAPI
DisconnectNamedPipe(
    IN HANDLE hNamedPipe
    );

WINBASEAPI
DWORD
WINAPI
GetLastError(
    VOID
    );

WINBASEAPI
BOOL_PARAM
WINAPI
WriteFile(
    IN HANDLE hFile,
    IN LPCVOID lpBuffer,
    IN DWORD nNumberOfBytesToWrite,
    OUT LPDWORD lpNumberOfBytesWritten,
    IN LPOVERLAPPED lpOverlapped
    );

WINBASEAPI
BOOL_PARAM
WINAPI
ReadFile(
    IN HANDLE hFile,
    OUT LPVOID lpBuffer,
    IN DWORD nNumberOfBytesToRead,
    OUT LPDWORD lpNumberOfBytesRead,
    IN LPOVERLAPPED lpOverlapped
    );

// #defines, structs and functions related to modality handling

typedef struct {
    UINT  cbSize;
    HWND  hwnd;
    DWORD dwFlags;
    UINT  uCount;
    DWORD dwTimeout;
} FLASHWINFO, *PFLASHWINFO;

#define FLASHW_CAPTION      0x00000001

WINUSERAPI
BOOL_PARAM
WINAPI
FlashWindowEx(
    PFLASHWINFO pfwi);

#define MB_OK                       0x00000000

WINUSERAPI
BOOL_PARAM
WINAPI
MessageBeep(
    IN UINT uType);

// Helpers for querying whether we're running on Vista
typedef struct _OSVERSIONINFOA {
    DWORD dwOSVersionInfoSize;
    DWORD dwMajorVersion;
    DWORD dwMinorVersion;
    DWORD dwBuildNumber;
    DWORD dwPlatformId;
    CHAR   szCSDVersion[ 128 ];     // Maintenance string for PSS usage
} OSVERSIONINFOA, *POSVERSIONINFOA, *LPOSVERSIONINFOA;

#define VER_PLATFORM_WIN32s             0
#define VER_PLATFORM_WIN32_WINDOWS      1
#define VER_PLATFORM_WIN32_NT           2

WINBASEAPI
BOOL_PARAM
WINAPI
GetVersionExA(
    IN OUT LPOSVERSIONINFOA lpVersionInformation
    );
