// Need more comments

/*********************************************************************************
Assumes the first character in configFileName passed to
FACE_IO_Initialize specifies whether this module lives
in the same program as the I/O Seg or in the PSS Seg.
  0 = The API is on the PSS side
  1 = The API is on the I/O Seg Side
  2 = I/O Seg and PSS are in the same program, Init was called by PSS
The rest of configFileName is a file name.
The header file associated with this C file is the FACE standard file ios.h
**********************************************************************************/

// In Visual, to set struct alignment to 1 byte, do:
//   Project -> Properties -> Configuration Properties -> C/C++ -> Code Generation -> Struct Memory Alignment -> 1 Byte
// Need to check for gcc


#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "globals.h"
#include "face_common.h"
#include "face_ios.h"
#include "config_parser.h"
#include "direct_call.h"

#ifdef LINUX_OS
   #include <sys/socket.h>
   #include <sys/select.h>
   #include <netinet/in.h>
   #include <arpa/inet.h>     // Otherwise get warning that inet_addr is implicitly defined.
#endif

#ifdef WINDOWS_OS
   #include <winsock2.h>
   #define s_addr S_un.S_addr         // Windows defines this differently than Posix
   #pragma comment(lib,"Ws2_32.lib")  // Get posix sockets in Windows - FACE doesn't allow pragma's
#endif



// Public Global variables.  
// Set these in I/O Seg if I/O Seg and PSS are in the same program,
// such as: IO_Seg_Initialize_Ptr = IO_Seg_Initialize;
// I don't like this but it is the best I can think of for now.
// This would be cleaner in C++.
// The externs for these and typedefs for the pointer types are in direct_call.h

IO_Seg_Initialize_PtrType IO_Seg_Initialize_Ptr;
IO_Seg_Read_PtrType IO_Seg_Read_Ptr;
IO_Seg_Write_PtrType IO_Seg_Write_Ptr;


typedef struct
{
   uint32_t               handle;            // API handle to use for Reads, Writes, etc. - 0 means not assigned
   int                    readSocket;        // Socket used to do Reads
   struct sockaddr_in     destAddr;          // Destination Address:  PSS for RX, I/O Seg for TX
   int                    writeSocket;       // Socket used to do Writes
   struct sockaddr_in     sourceAddr;        // Source Address:  We won't use for now - this would be for command/response
} CONNECTION_DATA_TYPE;


#define MAX_CONNECTIONS 10

// Parallel arrays to manage the connection
static FACE_CONFIG_DATA_TYPE configData[MAX_CONNECTIONS];
static CONNECTION_DATA_TYPE  connectionData[MAX_CONNECTIONS];


// Specifies whether this API is on the IO Segment side or not
static _Bool isItIOSeg;

// Number of Configured connections
static uint32_t numConnections = 0;

// Private Functions
static int InterfaceNameSearch(const char * name);
static _Bool CreateSockets(void);



/*************************************************************************
*********************  Public Methods ***********************************
*************************************************************************/


// Note that all must be reentrant, so all variables must be local.

// For the handles, I will simply use:  handle_num = slot_num + 1
// 0 will be reserved to mean an unused handle



// First character of the passed configuration_file
// must be a digit where:
//     0 = The API is on the PSS side
//     1 = The API is on the I/O Seg Side
//     2 = I/O Seg and PSS are in the same program, Init was called by PSS
// The rest is a file name, full path or relative to execution directory.
void FACE_IO_Initialize
   ( /* in */ const FACE_CONGIGURATION_FILE_NAME configuration_file,
     /* out */ FACE_RETURN_CODE_TYPE *return_code)
{
   uint32_t i;
   _Bool success;

   if ( configuration_file[0] != '0' && configuration_file[0] != '1' && configuration_file[0] != '2')
   {
      *return_code = FACE_INVALID_PARAM;
      return;
   }

   isItIOSeg = configuration_file[0] != '0';   // Assume yes except for '0'

   numConnections = MAX_CONNECTIONS;
   success = ParseConfigFile(configuration_file + 1, configData, &numConnections);

   for (i = 0; i < numConnections; i++)
   {
      connectionData[i].handle = 0;    // Initialize to unused
   }

   if(success)
   {
      if (configuration_file[0] == '2')
      {
         IO_Seg_Initialize_Ptr (configuration_file, return_code);
         if (*return_code != FACE_NO_ERROR)
         {
            return;
         }
      }
      CreateSockets();                // Not sure what to do if this fails - no FACE_RETURN_CODE_TYPE seems appropriate

      *return_code = FACE_NO_ERROR;
   }
   else
   {
      *return_code = FACE_INVALID_CONFIG;
   }
}

void FACE_IO_Open
   ( /* in */ const FACE_INTERFACE_NAME_TYPE name,
     /* out */ FACE_INTERFACE_HANDLE_TYPE *handle,
     /* out */ FACE_RETURN_CODE_TYPE *return_code)
{

   int index = InterfaceNameSearch(name);

   if(index < 0)
   {
      *return_code = FACE_INVALID_PARAM;
   }
   else
   {
      if(connectionData[index].handle > 0 )
      {
         *return_code = FACE_ADDR_IN_USE;
      }
      else
      {
         // For now, the handle will simply be the index+1 in the array.
         // That can change if a more complicated scheme is needed.
         *return_code = FACE_NO_ERROR;
         *handle = (void *)((long)(index + 1));   // / Need double cast to remove warnings from IO and Windows
         connectionData[index].handle = index + 1;
      }
   }
}

void FACE_IO_Register
   ( /* in */ FACE_INTERFACE_HANDLE_TYPE handle,
   /* in */ FACE_CALLBACK_ADDRESS_TYPE callback_address,
   /* out */ FACE_RETURN_CODE_TYPE *return_code)
{
   // Don't implement for now.
   *return_code = FACE_NOT_AVAILABLE;
}

void FACE_IO_Read
   ( /* in */ FACE_INTERFACE_HANDLE_TYPE handle,
     /* in */ FACE_TIMEOUT_TYPE timeout,
     /* inout */ FACE_MESSAGE_LENGTH_TYPE *message_length,
     /* in */ FACE_MESSAGE_ADDR_TYPE data_buffer_address,
     /* out */ FACE_RETURN_CODE_TYPE *return_code)
{
   uint32_t iHandle = (uint32_t)((long)handle);      // Need double cast to remove warnings from IO and Windows                  
   int index = iHandle - 1;                          // Handle is one more than the array possition

   if (iHandle > numConnections || iHandle <= 0)
   {
      *return_code = FACE_INVALID_PARAM;
   }
   else if (connectionData[index].handle <= 0)
   {
      *return_code = FACE_CONNECTION_CLOSED;
   }
   else if (configData[index].connectionType == FACE_DIRECT_CONNECTION)
   {
      IO_Seg_Read_Ptr(message_length, data_buffer_address, return_code);
   }
   else if (configData[index].connectionType == FACE_UDP_CONNECTION)
   {
      int32_t rcvResult;
      fd_set fdset;          // Set up file descriptors to monitor (just this one)
      int selResult;
      struct timeval timeoutTime;

      // Set it up to use the timeout
      FD_ZERO(&fdset);
      FD_SET(connectionData[index].readSocket, &fdset);           // monitor this one
      timeoutTime.tv_sec = (long)(timeout / 1000000000UL);
      timeoutTime.tv_usec = (timeout % 1000000000UL) / 1000;      // Convert nanoseconds to microseconds
      selResult = select(connectionData[index].readSocket + 1,    // Highest descriptor + 1
         &fdset,                                                  // input set
         NULL,                                                    // output set
         NULL,                                                    // error set
         &timeoutTime);                                           // timeout

      if(selResult <= 0)
      {
         // printf("Read debug line - legit time out %d\n", selResult);
         *return_code = FACE_TIMED_OUT;
      }
      else
      {
         rcvResult = recvfrom( connectionData[index].readSocket,
                               (char *)data_buffer_address, *message_length, 0, NULL, NULL);
         if(rcvResult <= 0)
         {
            *return_code = DATA_BUFFER_TOO_SMALL;   // There are other errors it could be, they could be checked
         }
         else
         {
            *message_length = rcvResult;
            *return_code = FACE_NO_ERROR;
         }
      }
   }
   else
   {
      // Unknown FACE_CONNECTION_TYPE
      *return_code = FACE_INVALID_PARAM;
   }
}


void FACE_IO_Write
   ( /* in */ FACE_INTERFACE_HANDLE_TYPE handle,
     /* in */ FACE_TIMEOUT_TYPE timeout,
     /* in */ FACE_MESSAGE_LENGTH_TYPE message_length,
     /* in */ FACE_MESSAGE_ADDR_TYPE data_buffer_address,
     /* out */ FACE_RETURN_CODE_TYPE *return_code)
{

   uint32_t iHandle = (uint32_t)((long)handle);      // Need double cast to remove warnings from IO and Windows 
   int index = iHandle - 1;                          // Handle is one more than the array position

      if( iHandle > numConnections )
   {
      *return_code = FACE_INVALID_PARAM;
   }
   else if ( connectionData[index].handle <= 0 )
   {
      *return_code = FACE_CONNECTION_CLOSED;
   }
   else if (configData[index].connectionType == FACE_DIRECT_CONNECTION)
   {
      IO_Seg_Write_Ptr(message_length, data_buffer_address, return_code);
   }
   else if (configData[index].connectionType == FACE_UDP_CONNECTION)
   {
      int32_t len;
      int sendResult;
      fd_set fdset;
      int selResult;
      struct timeval timeoutTime;

      // Set it up to use the timeout
      FD_ZERO(&fdset);
      FD_SET(connectionData[index].writeSocket, &fdset);           // monitor this one

      timeoutTime.tv_sec = (long)(timeout / 1000000000UL);
      timeoutTime.tv_usec = (timeout % 1000000000UL) / 1000;       // Convert nanoseconds to microseconds
      selResult = select(connectionData[index].writeSocket + 1,    // Highest descriptor + 1
         NULL,           // input set
         &fdset,         // output set
         NULL,           // error set
         &timeoutTime);  // timeout
      if(selResult <= 0)
      {
         *return_code = FACE_TIMED_OUT;
      }
      else
      {
         len = sizeof(connectionData[index].destAddr);
         sendResult = sendto( connectionData[index].writeSocket, (char *)data_buffer_address, message_length, 0,
            (struct sockaddr *)&connectionData[index].destAddr, len);
         if(sendResult < 0)
         {
            *return_code = FACE_TIMED_OUT;  // None really match
         }
         else
         {
            *return_code = FACE_NO_ERROR;
         }
      }
   }
   else
   {
      // Unknown FACE_CONNECTION_TYPE
      *return_code = FACE_INVALID_PARAM;
   }
}


void FACE_IO_Get_Status
   ( /* in */ FACE_INTERFACE_HANDLE_TYPE handle,
   /* out */ FACE_STATUS_TYPE *status,
   /* out */ FACE_RETURN_CODE_TYPE *return_code)
{
   // Don't implement for now.
   *return_code = FACE_NOT_AVAILABLE;
}


void FACE_IO_Close
   ( /* in */ FACE_INTERFACE_HANDLE_TYPE handle,
   /* out */ FACE_RETURN_CODE_TYPE *return_code)
{
   uint32_t iHandle = (uint32_t)((long)handle);      // Need double cast to remove warnings from IO and Windows 
   int index = iHandle - 1;                          // Handle is one more than the array possition

   if (iHandle > 0 && iHandle <= numConnections)
   {
      connectionData[index].handle = 0;
      *return_code = FACE_NO_ERROR;
   }
   else
   {
      *return_code = FACE_INVALID_PARAM;
   }
}


/*************************************************************************
*********************  Private Methods ***********************************
*************************************************************************/


// Searches nameToPort to find if index of name.  Returns -1 if name not found.
static int InterfaceNameSearch(const char * name)
{
   uint32_t i;

   for(i = 0; i < numConnections; ++i)
   {
      if (strncmp(name, configData[i].name, sizeof(FACE_INTERFACE_NAME_TYPE)) == 0)
      {
         return i;
      }
   }
   return -1;
}

// The PSS calls Write for TX, Read for RX.  I/O Seg calls Read for TX, Write for RX.
// These four combinations determine how  connectionData[i].readSocket & writeSoecket are set.
// connectionData[i].destAddr is PSS for RX, IO Seg for TX, and is used for Writes.
// So only one side uses it per connection.
// For now, don't handle Command/Response, so assume connectionData[i].sourceAddr is not used.
// So that means sourceAddress, sourcePort are not used.

static _Bool CreateSockets(void)
{
   uint32_t i;
   int result;
   _Bool retRes = true;

#ifdef WINDOWS_OS
   // Didn't see this one coming!  Not sure how to make this standard Posix!!!
   WORD wVersionRequested;
   WSADATA wsaData;
   int err;

    wVersionRequested = MAKEWORD(2, 2);    // Use the MAKEWORD(lowbyte, highbyte) macro declared in Windef.h
    err = WSAStartup(wVersionRequested, &wsaData);
    if (err != 0) 
    {
        printf("I/O Lib Error: Windows Sockets failed start-up with error: %d\n", err);
        return false;
    }
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) 
    {
        printf("Could not find a usable version of Winsock.dll\n");
        WSACleanup();
        return false;
    }
#endif

   for (i = 0; i < numConnections; ++i)
   {
      if (configData[i].connectionType == FACE_UDP_CONNECTION)
      {
         if (configData[i].destinationAddress[0] == '\0' || configData[i].destinationPort == '\0')
         {
            printf("I/O Lib Error:  Missing Destination data in XML file for connection %s\n", configData[i].name);
            retRes = false;
         }
         else
         {
            // Create the Read and Write Sockets
            connectionData[i].readSocket = socket(AF_INET, SOCK_DGRAM, 0);
            if (connectionData[i].readSocket < 0)
            {
                printf("I/O Lib Error:  Couldn't create socket for Connection %s.  Error is: %d\n",
                    configData[i].name, connectionData[i].readSocket);
                perror("Error: ");
                retRes = false;
            }
            connectionData[i].writeSocket = socket(AF_INET, SOCK_DGRAM, 0);
            if (connectionData[i].writeSocket < 0)
            {
                printf("I/O Lib Error:  Couldn't create socket for Connection %s.  Error is: %d\n",
                    configData[i].name, connectionData[i].writeSocket);
                perror("Error: ");
                retRes = false;
            }

            // Start the setup of the Destination Address
            memset(&connectionData[i].destAddr, 0, sizeof(connectionData[i].destAddr));
            connectionData[i].destAddr.sin_family = AF_INET;
            connectionData[i].destAddr.sin_port =  htons(configData[i].destinationPort);

            if ((isItIOSeg && configData[i].direction == FACE_TRANSMIT) ||
               (!isItIOSeg && configData[i].direction == FACE_RECEIVE))
            {
               // The destination is on the "Read" side, so need to bind the address to the read socket 
               connectionData[i].destAddr.sin_addr.s_addr = 0;      // htonl(INADDR_ANY);
               result = bind(connectionData[i].readSocket,
                    (struct sockaddr *)&connectionData[i].destAddr,
                    sizeof(connectionData[i].destAddr));
                if ( result < 0 )
                {
                    printf("I/O Lib Error:  Couldn't Bind  %s.  Error is: %d\n",
                        configData[i].name, result);
                    perror("Error: ");
                    retRes = false;
                }
            }
            else
            {
               // The destination is on the "Write" side, so the destinatino address is where the sendTo goes
               connectionData[i].destAddr.sin_addr.s_addr = inet_addr(configData[i].destinationAddress);
            }
         }
      }
   }

   return retRes;
}




//********************************************************************************************************************
//********************************************************************************************************************
//*******************  TESTBED MAIN  *********************************************************************************
//********************************************************************************************************************
//********************************************************************************************************************

// Use these as samples of what you will need in the PSS and I/O Seg.

//#define TESTING_A429
#ifdef TESTING_A429


// Comment out exactly one of these
#define TEST_AS_IO_SEG
//#define TEST_AS_PSS


#include <stdio.h>

#ifdef LINUX_OS
   #include <unistd.h>
#endif

#include "face_messages.h"

#define MAX_BUFF_SIZE  1024

int main()
{
   int i;
   FACE_RETURN_CODE_TYPE retCode;
   FACE_INTERFACE_HANDLE_TYPE dataLogHandle;
   FACE_INTERFACE_HANDLE_TYPE igsHandle;
   int dataLogIndex;
   int igsIndex;
   FACE_MESSAGE_LENGTH_TYPE messLen;

   char txBuff[MAX_BUFF_SIZE];
   char rxBuff[MAX_BUFF_SIZE];

   FACE_IO_MESSAGE_TYPE * txFaceMsg = (FACE_IO_MESSAGE_TYPE *)txBuff;
   FACE_IO_MESSAGE_TYPE * rxFaceMsg = (FACE_IO_MESSAGE_TYPE *)rxBuff;
   FACE_A429_MESSAGE_TYPE * txA429Data = (FACE_A429_MESSAGE_TYPE *)txFaceMsg->data;
   FACE_A429_MESSAGE_TYPE * rxA429Data = (FACE_A429_MESSAGE_TYPE *)rxFaceMsg->data;
   
   // Zero them out
   memset(txBuff, 0, MAX_BUFF_SIZE);
   memset(rxBuff, 0, MAX_BUFF_SIZE);

   // Set the fixed fields - make sure to convert to network order when needed
   txFaceMsg->guid = htonl(100);
   rxFaceMsg->guid = htonl(200);   // Pick a couple of numbers
   txFaceMsg->busType =  FACE_ARINC_429;
   rxFaceMsg->busType = FACE_ARINC_429;
   txFaceMsg->message_type = htons(FACE_DATA);
   rxFaceMsg->message_type = htons(FACE_DATA);

   printf("Starting testbed main for I/O API\n");

   // Call Init
#ifdef TEST_AS_IO_SEG
   printf("I/O API - Test bed acting as IO Seg\n");
   FACE_IO_Initialize("1config.xml", &retCode);
//   FACE_IO_Initialize("1configLocalHost.xml", &retCode);
#endif
#ifdef TEST_AS_PSS
   printf("I/O API - Test bed acting as PSS\n");
   FACE_IO_Initialize("0config.xml", &retCode);
//   FACE_IO_Initialize("0configLocalHost.xml", &retCode);
#endif

   if (retCode == FACE_NO_ERROR)
   {
      printf("I/O API - Config file parsed okay.  Number of devices is %d\n", numConnections);
   }
   else
   {
      printf("I/O API - Error parsing config file.\n");
   }

   // Use the indices to get at the Config data to make up the messages
   dataLogIndex = InterfaceNameSearch("A429_TX_DATALOGGER");
   igsIndex = InterfaceNameSearch("A429_RX_IGS");
   
   // Open the connections.
   FACE_IO_Open("A429_TX_DATALOGGER", &dataLogHandle, &retCode);
   printf("I/O API - Return codes for Open are datalogger %d ", retCode);
   FACE_IO_Open("A429_RX_IGS", &igsHandle, &retCode);
   printf("IGS %d\n", retCode);

#ifdef WINDOWS_OS
   Sleep(7000);  // Give me time to start both sides up
#endif

#ifdef LINUX_OS
   sleep(7);  // Give me time to start both sides up
#endif

   // Read and write a message
#ifdef TEST_AS_IO_SEG
   printf("I/O API Test bed - Pretend to be IO Seg - send and receive a message\n");

   for (i = 0; i < 3; i++)
   {
      static uint32_t dummyData1 = 0x12345678;  // Not a valid Arinc - I am just testing
      static uint32_t dummyData2 = 0x98765432;  // Not a valid Arinc - I am just testing

      // Send a IGS Message
      FaceSetPayLoadLength(txFaceMsg,sizeof(FACE_A429_MESSAGE_TYPE) + 4);  // Send two words
      txA429Data->channel = (uint8_t)(configData[igsIndex].channel);
      txA429Data->num_labels = 2;
      txA429Data->data[0] = htonl(dummyData1++);
      txA429Data->data[1] = htonl(dummyData2++);
      FACE_IO_Write(igsHandle, 0, FACE_MSG_HEADER_SIZE + FacePayLoadLength(txFaceMsg), txFaceMsg, &retCode);
      if (retCode == FACE_NO_ERROR)
         printf ("I/O API Test bed - IO Seg just wrote IGS with no error.\n");
      else
         printf ("I/O API Test bed - IO Seg just wrote IGS with error: %d.\n", retCode);
   
      // Read the Data Logger Message
      messLen = FACE_MSG_HEADER_SIZE + sizeof(FACE_A429_MESSAGE_TYPE);
      FACE_IO_Read(dataLogHandle, 5000000000LL, &messLen, rxFaceMsg, &retCode);   // Give it time for other side to send
      if (retCode == FACE_NO_ERROR)
         printf ("I/O API Test bed - IO Seg just read DataLog no error.  First A429 Word was: %0X.\n", ntohl(rxA429Data->data[0]));
      else
         printf ("I/O API Test bed - IO Seg just read DataLog with error: %d.\n", retCode);
   }

#endif
   
#ifdef TEST_AS_PSS
   printf("I/O API Test bed - Pretend to be PSS - send and receive a message\n");

   for (i = 0; i < 3; i++)
   {
      static uint32_t dummyData = 0xABCDEF01;   // Not a valid Arinc - I am just testing
      // Send a Data Logger Message
      FaceSetPayLoadLength(txFaceMsg,sizeof(FACE_A429_MESSAGE_TYPE));  // Send one word
      txA429Data->channel = (uint8_t)(configData[igsIndex].channel);
      txA429Data->num_labels = 1;
      txA429Data->data[0] = htonl(dummyData++);
      FACE_IO_Write(dataLogHandle, 0, FACE_MSG_HEADER_SIZE + FacePayLoadLength(txFaceMsg), txFaceMsg, &retCode);
      if (retCode == FACE_NO_ERROR)
         printf ("I/O API Test bed - IO Seg just wrote DataLog with no error.\n");
      else
         printf ("I/O API Test bed - IO Seg just wrote DataLog with error: %d.\n", retCode);
   
      // Read the IGS Message
      messLen = FACE_MSG_HEADER_SIZE + sizeof(FACE_A429_MESSAGE_TYPE) + 4;
      FACE_IO_Read(igsHandle, 5000000000LL, &messLen, rxFaceMsg, &retCode);   // Give it time for other side to send
      if (retCode == FACE_NO_ERROR)
         printf ("I/O API Test bed - IO Seg just read IGS no error.  A429 Words were: %0X %0X.\n", ntohl(rxA429Data->data[0]), ntohl(rxA429Data->data[1]));
      else
         printf ("I/O API Test bed - IO Seg just read IGS with error: %d.\n", retCode);
   }

#endif

   {
      char ch;
      printf ("Press any key followed by Enter to continue\n", retCode);
      scanf("%c", &ch);
   }

   return 0;
}


#endif 


/************************************************************************************************************************/
/************************************************************************************************************************/
/************************************************************************************************************************/
/************************************************************************************************************************/
//#define TESTING_DISCRETES
#ifdef TESTING_DISCRETES

#include <stdio.h>
#include "face_messages.h"

// Declare externs for these
extern IO_Seg_Initialize_PtrType IO_Seg_Initialize_Ptr;
extern IO_Seg_Read_PtrType IO_Seg_Read_Ptr;
extern IO_Seg_Write_PtrType IO_Seg_Write_Ptr;

#define MAX_BUFF_SIZE  1024

void IO_Seg_Initialize
     ( /* in */ const FACE_CONGIGURATION_FILE_NAME configuration_file,
       /* out */ FACE_RETURN_CODE_TYPE *return_code)
{
   // Needs to be written by IO Seg.  Will parse the file 
   *return_code = FACE_NO_ERROR;
}


void IO_Seg_Read
   ( /* inout */ FACE_MESSAGE_LENGTH_TYPE *message_length,
     /* in */ FACE_MESSAGE_ADDR_TYPE data_buffer_address,
     /* out */ FACE_RETURN_CODE_TYPE *return_code)
{
   static uint8_t bitVal = 0;
   
   FACE_IO_MESSAGE_TYPE * faceMsg = (FACE_IO_MESSAGE_TYPE *)data_buffer_address;
   printf("In IO Seg Read.  ");

   if (faceMsg->busType == FACE_DISCRETE)
   {
      printf("Need to read channel %d.  ", FaceDiscreteChannelNumber(faceMsg));
      printf("  Going to return a %d\n", bitVal);
      FaceSetDiscreteState(faceMsg, bitVal);
      bitVal = 1 - bitVal;                     // Change it for next time
      *return_code = FACE_NO_ERROR;
   }
   else
   {
      printf("Can't handle the bustype sent: %d.\n", faceMsg->busType);
      *return_code = FACE_INVALID_PARAM;
   }
}

// For Direct Write when PSS and I/O Seg are in the same program
void IO_Seg_Write
   (  /* in */ FACE_MESSAGE_LENGTH_TYPE message_length,
      /* in */ FACE_MESSAGE_ADDR_TYPE data_buffer_address,
      /* out */ FACE_RETURN_CODE_TYPE *return_code)
{
   FACE_IO_MESSAGE_TYPE * faceMsg = (FACE_IO_MESSAGE_TYPE *)data_buffer_address;

   printf("In IO Seg Write.  ");
   if (faceMsg->busType == FACE_DISCRETE)
   {
      printf("Need to write channel %d with a %d.\n", FaceDiscreteChannelNumber(faceMsg), FaceDiscreteState(faceMsg));
      *return_code = FACE_NO_ERROR;
   }
   else
   {
      printf("Can't handle the bustype sent: %d.\n", faceMsg->busType);
      *return_code = FACE_INVALID_PARAM;
   }
}


int main()
{
   int i;
   FACE_RETURN_CODE_TYPE retCode;
   static uint8_t bitVal = 1;
   FACE_INTERFACE_HANDLE_TYPE wowHandle;
   FACE_INTERFACE_HANDLE_TYPE emergencyHandle;
   int wowIndex;
   int emergencyIndex;
   FACE_MESSAGE_LENGTH_TYPE messLen;
   

   char txBuff[MAX_BUFF_SIZE];
   char rxBuff[MAX_BUFF_SIZE];

   FACE_IO_MESSAGE_TYPE * txFaceMsg = (FACE_IO_MESSAGE_TYPE *)txBuff;
   FACE_IO_MESSAGE_TYPE * rxFaceMsg = (FACE_IO_MESSAGE_TYPE *)rxBuff;


   // Set up the functions pointers to call the direct calls 
   // since IO Seg and PSS are in the same program.
   IO_Seg_Initialize_Ptr = IO_Seg_Initialize;
   IO_Seg_Read_Ptr = IO_Seg_Read;
   IO_Seg_Write_Ptr = IO_Seg_Write;


   // Zero them out
   memset(txBuff, 0, MAX_BUFF_SIZE);
   memset(rxBuff, 0, MAX_BUFF_SIZE);

   // Set the fixed fields - make sure to convert to network order when needed
   txFaceMsg->guid = htonl(100);
   rxFaceMsg->guid = htonl(200);   // Pick a couple of numbers
   txFaceMsg->busType = FACE_DISCRETE;
   rxFaceMsg->busType = FACE_DISCRETE;
   txFaceMsg->message_type = htons(FACE_DATA);
   rxFaceMsg->message_type = htons(FACE_DATA);
   FaceSetPayLoadLength(txFaceMsg, 4);
   FaceSetPayLoadLength(rxFaceMsg, 4);

   printf("Starting testbed main for I/O API\n");

   // Call Init
   FACE_IO_Initialize("1config.xml", &retCode);

   if (retCode == FACE_NO_ERROR)
   {
      printf("Config file parsed okay.  Number of devices is %d\n", numConnections);
   }
   else
   {
      printf("Error parsing config file.\n");
   }

   // Get the channel numbers from config and set them in the messages
   wowIndex = InterfaceNameSearch("DISCRETE_INPUT_WOW");
   emergencyIndex = InterfaceNameSearch("DISCRETE_OUTPUT_EMERGENCY");
   FaceSetDiscreteChannelNumber(txFaceMsg, configData[emergencyIndex].channel);
   FaceSetDiscreteChannelNumber(rxFaceMsg, configData[wowIndex].channel);


   // Open the connections.
   FACE_IO_Open("DISCRETE_INPUT_WOW", &wowHandle, &retCode);
   FACE_IO_Open("DISCRETE_OUTPUT_EMERGENCY", &emergencyHandle, &retCode);


   // Read and write a few of messages, pretending to be PSS
   for (i = 0; i < 3; i++)
   {
      // Transmit
      FaceSetDiscreteState(txFaceMsg, bitVal);
      printf ("PSS going to to write a %d to Emergency channel number %d.\n", bitVal, FaceDiscreteChannelNumber(txFaceMsg));
      FACE_IO_Write(emergencyHandle, 0, FACE_MSG_HEADER_SIZE + 4, txFaceMsg, &retCode);
      if (retCode == FACE_NO_ERROR)
      {
          printf ("PSS Write worked.\n", bitVal);
      }
      else
      {
          printf ("PSS Write failed with: %d.\n", retCode);
      }
      bitVal = 1 - bitVal;

      // Receive
      printf ("PSS going to try to read WOW on channel %d.\n", FaceDiscreteChannelNumber(rxFaceMsg));
      messLen = FACE_MSG_HEADER_SIZE + 4;
      FACE_IO_Read(wowHandle, 0, &messLen, rxFaceMsg, &retCode);
      if (retCode == FACE_NO_ERROR)
      {
          printf ("PSS Read worked.  Value read was %d.  Number of bytes in message was: %d.\n", FaceDiscreteState(rxFaceMsg), messLen);
      }
      else
      {
          printf ("PSS Write failed with: %d.\n", retCode);
      }
   }

   FACE_IO_Close(wowHandle, &retCode);
   FACE_IO_Close(emergencyHandle, &retCode);

   return 0;
}


#endif
