#include <string.h>
#include <stdio.h>
#include "face_pss.h"
#include "config_parser.h"

#ifdef COLOR
#define FAILMESSAGE "\x1b[31mFAILED\x1b[39m\n"
#define GOODMESSAGE "\x1b[32mSUCCESS\x1b[39m\n"
#else
#define FAILMESSAGE "FAILED\n"
#define GOODMESSAGE "SUCCESS\n"
#endif

#define MAX_ARINC_LABELS_PER_MESSAGE 10

static int getHandleIndex(FACE_CONFIG_DATA_TYPE config[], int channel, int/*XXX*/ bustype)
{
   int i;
   for(i = 0; i < MAX_CONNECTIONS; i++)
   {
      if(config[i].channel == channel && config[i].busType == bustype)
      {
         return i;//handles[i];
      }
   }
   printf("invalid channel!\n");
   return -1;
}

void repl(FACE_INTERFACE_HANDLE_TYPE handles[], FACE_CONFIG_DATA_TYPE config[])
{
	printf("h for help\n");
	while(1)
	{
		
		char command;
		int channel = -1;
		printf(">");
		if(scanf("%c", &command) == EOF)
		{
			printf("\n");
			return;
		}
		if(command == 'h')
		{
			printf("s 5 -- set discrete 5\n");
			printf("c 5 -- clear discrete 5\n");
			//printf("z 5 -- high impedance discrete 5\n");
			printf("r 5 -- read discrete 5\n");
			printf("w 5 1 0xf4532323 -- send 0xf4532323 on ARINC channel 5\n");
			printf("w 5 3 0xf4532323 0xf4532323 0xf4532323 -- send 3 words on ARINC channel 5\n");
			printf("a 5 3 0xf4532323 0xf4532323 0xf4532323 -- receive 3 words from ARINC channel 5\n");
			printf("q   -- quit\n");
			printf("h   -- help\n");
			scanf("%c", &command);//discard the character so we don't get >> (I don't like this)
			continue;
		}
		if(command == 'q')
			return;
		if(command == '\n')
			continue;
		if(command != 's' && command != 'c' && command != 'r' && command != 'a' && command != 'w' )
		{
			while(command != '\n')
				scanf("%c", &command);
			continue;
		}
		scanf("%d", &channel);
		//test if channel is available here?
		if(command == 's')
		{
			FACE_RETURN_CODE_TYPE result = FACE_NO_ERROR;
			printf("set channel %d: ", channel);
			int handleid = getHandleIndex(config, channel, FACE_DISCRETE);
			if(handleid > 0 && config[handleid].direction != FACE_TRANSMIT)
				printf("channel not transmit capable!%s\n", FAILMESSAGE);
			else if(handleid < 0)
				printf("channel does not exist!%s\n", FAILMESSAGE);
			else
			{
//				setDiscrete(handles[handleid], channel,  1, &result);
				printf(result != FACE_NO_ERROR?FAILMESSAGE:GOODMESSAGE);
			}
		}
		if(command == 'c')
		{
			FACE_RETURN_CODE_TYPE result = FACE_NO_ERROR;
			printf("clear channel %d: ", channel);
			int handleid = getHandleIndex(config, channel, FACE_DISCRETE);
			if(handleid > 0 && config[handleid].direction != FACE_TRANSMIT)
				printf("channel not transmit capable!%s\n", FAILMESSAGE);
			else if(handleid < 0)
				printf("channel does not exist!%s\n", FAILMESSAGE);
			else
			{
//				setDiscrete(handles[handleid], channel,  0, &result);
				printf(result != FACE_NO_ERROR?FAILMESSAGE:GOODMESSAGE);
			}
		}
		if(command == 'w')
		{
			uint32_t numwords;
			scanf("%u", &numwords);
			int handleid = getHandleIndex(config, channel, FACE_ARINC_429);
			if(handleid < 0 )
				;//printf("Invalid channel!\n");
			else if(numwords > MAX_ARINC_LABELS_PER_MESSAGE)
			{
				uint32_t dummy;
				int i;
				printf("Too many words (max is %d)\n", MAX_ARINC_LABELS_PER_MESSAGE);
				for(i = 0; i < numwords; i ++)
					scanf("%x", &dummy);
			}
			else
			{
				FACE_RETURN_CODE_TYPE result = FACE_NO_ERROR;
				uint32_t arincwords[MAX_ARINC_LABELS_PER_MESSAGE+1];
				int i;
				for(i = 0; i < numwords; i ++)
					scanf("%x", &arincwords[i]);
				sendArinc429(handles[handleid], (uint8_t)channel, arincwords, numwords, &result);
				printf("%d\n", result);
				printf(result != FACE_NO_ERROR?FAILMESSAGE:GOODMESSAGE);
			}
		}
		if(command == 'a')
		{
			uint32_t numwords;
			scanf("%u", &numwords);
			int handleid = getHandleIndex(config, channel, FACE_ARINC_429);
			if(handleid < 0 )
				;//printf("Invalid channel!\n");
			else if(numwords > MAX_ARINC_LABELS_PER_MESSAGE)
			{
				printf("Too many words (max is %d)\n", MAX_ARINC_LABELS_PER_MESSAGE);
			}
			else
			{
				FACE_RETURN_CODE_TYPE result = FACE_NO_ERROR;
				uint32_t arincwords[MAX_ARINC_LABELS_PER_MESSAGE+1];
				int i;
				uint8_t channeldummy;
				readArinc429(handles[handleid], &channeldummy, arincwords, &numwords, &result);
				for(i = 0; i < numwords && result == FACE_NO_ERROR; i ++)
					printf(" 0x%x ", arincwords[i]);
				printf(result != FACE_NO_ERROR?FAILMESSAGE:GOODMESSAGE);
				printf("%d", result);
			}
		}
		if(command == 'r')
		{
			FACE_RETURN_CODE_TYPE result = FACE_NO_ERROR;
			int value = -1;
			printf("read channel %d:", channel);
			int handleid = getHandleIndex(config, channel, FACE_DISCRETE);
			if(handleid > 0 && config[handleid].direction != FACE_RECEIVE)
				printf("channel not receive capable!%s\n", FAILMESSAGE);
			else if(handleid < 0)
				printf("channel does not exist!%s\n", FAILMESSAGE);
			else
			{
//				value = readDiscrete(handles[handleid], channel, &result);
				printf(result != FACE_NO_ERROR?FAILMESSAGE:GOODMESSAGE);
				printf("value of channel %d:%d\n", channel, value); 
			}
		}
		scanf("%c", &command);//discard the character so we don't get >> (I don't like this)
	}

}

#ifdef TESTREPL
int main(){repl(NULL);}
#endif

#define MAX_LABELS 1024

void *readConnection(void *handlePtr)
{
   FACE_INTERFACE_HANDLE_TYPE handle = *(FACE_INTERFACE_HANDLE_TYPE*)handlePtr;
   uint8_t channel = 0;
   uint32_t data[MAX_LABELS];
   uint32_t labels = MAX_LABELS;
   FACE_RETURN_CODE_TYPE retCode;
   
   printf("Started read thread.\n");

   while(1)
   {
      // This probably doesn't have to be done each time
      memset(data, 0, MAX_LABELS);

      readArinc429(handle, &channel, data, &labels, &retCode);

      if(retCode == FACE_NO_ERROR)
      {
         // Got some data, now print it out
         uint32_t i;
         printf("Got the following labels for channel %u:\n", channel);
         for(i = 0; i < labels; i++)
         {
            printf("0x%X\n", data[i]);
         }
      }
   }
   
   return NULL;
}
