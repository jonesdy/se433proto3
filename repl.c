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

static int gethandle(FACE_CONFIG_DATA_TYPE config[], int channel)
{
   int i;
   for(i = 0; i < MAX_CONNECTIONS; i++)
   {
      if(config[i].channel == channel && config[i].busType == FACE_DISCRETE)
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
			printf("q   -- quit\n");
			printf("h   -- help\n");
			scanf("%c", &command);//discard the character so we don't get >> (I don't like this)
			continue;
		}
		if(command == 'q')
			return;
		if(command == '\n')
			continue;
		if(command != 's' && command != 'c' && command != 'r')
		{
			while(command != '\n')
				scanf("%c", &command);
			continue;
		}
		scanf("%d", &channel);
		//test if channel is available here?
		if(command == 's')
		{
			FACE_RETURN_CODE_TYPE result;
			printf("set channel %d: ", channel);
			int handleid = gethandle(config, channel);
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
			FACE_RETURN_CODE_TYPE result;
			printf("clear channel %d: ", channel);
			int handleid = gethandle(config, channel);
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
		if(command == 'r')
		{
			FACE_RETURN_CODE_TYPE result;
			int value = -1;
			printf("read channel %d:", channel);
			int handleid = gethandle(config, channel);
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
