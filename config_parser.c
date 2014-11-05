// Need a comment block


#include "globals.h"
#include "config_parser.h"

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

#define MAX_LINE_LEN 120


// Local functions.
static void StoreString (const char * src, char * dest);
static void StoreLongLong(const char * src, uint64_t * dest);
static void StoreShort(const char * src, uint16_t * dest);
static void StoreByte(const char * src, uint8_t * dest);



// Parses a config file and returns the config array and numbe of connections.
// The user must pass a pointer to an array of CONNECTION_INFO_TYPE and maximum number of elements in the array.
// The function passes back the connection data in the array and the number of connections.
// It returns true on success, false on failure.
_Bool PasrseConfigFile( /* in */ const char * filename, /*out */ FACE_CONFIG_DATA_TYPE config[], /*in out */ uint32_t * numConnections)
{
   char line[MAX_LINE_LEN];
   int index;
   _Bool firstTime = true;
   uint32_t maxConnections = *numConnections;
   FILE *infile;

   infile = fopen(filename,"r");
   if (infile == 0) 
   {
      printf("XML Error: Cannot open input file '%s'.\n", filename); 
      return false;
   }

   // Zero out all the memory
   memset(config, 0, maxConnections * sizeof(FACE_CONFIG_DATA_TYPE));

   *numConnections = 0;     // For now, it is where the data will be stored
   while (fgets(line, MAX_LINE_LEN, infile) != NULL  && *numConnections < maxConnections)
   {
      // Brute force it - will get rather long when it's done.
      // I won't parse whole lines, just enough to get out the needed info.
      // I aslo assume one item per line, item doesn't cross lines, no leading spaces, etc. - that everything is "nice"

      if (strncmp("<connection>", line, strlen("<connection>")) == 0)
      {
         if (!firstTime)
         {
            ++*numConnections;   // Start the next connection
         }
         else
         {
            firstTime = false;
         }
      }
      else if (strncmp("<name>", line, strlen("<name>")) == 0)
      {
         StoreString(line + strlen("<name>"), config[*numConnections].name);
      }
      else if (strncmp("<type>", line, strlen("<type>")) == 0)
      {
         index = strlen("<type>");
         if (tolower(line[index]) == 'd')
            config[*numConnections].connectionType = FACE_DIRECT_CONNECTION;
         else if (tolower(line[index]) == 'u')
            config[*numConnections].connectionType = FACE_UDP_CONNECTION;
      }
      else if (strncmp("<direction>", line, strlen("<direction>")) == 0)
      {
         index = strlen("<direction>");
         if (tolower(line[index]) == 't')
            config[*numConnections].direction = FACE_TRANSMIT;
         else if (tolower(line[index]) == 'r')
            config[*numConnections].direction = FACE_RECEIVE;
      }
      else if (strncmp("<refreshperiod>", line, strlen("<refreshperiod>")) == 0)
      {
         StoreLongLong(line + strlen("<refreshperiod>"), &config[*numConnections].refreshPeriod);
      }
      else if (strncmp("<iotype>", line, strlen("<iotype>")) == 0)
      {
         index = strlen("<iotype>");
         if (tolower(line[index]) == 'a' && tolower(line[index + 1]) == '4')
            config[*numConnections].busType = FACE_ARINC_429;
         else if (tolower(line[index]) == 'd')
            config[*numConnections].busType = FACE_DISCRETE;
      }
      else if (strncmp("<channel>", line, strlen("<channel>")) == 0)
      {
         StoreShort(line + strlen("<channel>"), &config[*numConnections].channel);
      }
      else if (strncmp("<discrete_inital_value>", line, strlen("<discrete_inital_value>")) == 0)
      {
         StoreByte(line + strlen("<discrete_inital_value>"), &config[*numConnections].discreteInitialValue);
      }
      else if (strncmp("<destinationaddress>", line, strlen("<destinationaddress>")) == 0)
      {
         StoreString(line + strlen("<destinationaddress>"), config[*numConnections].destinationAddress);
      }
      else if (strncmp("<destinationport>", line, strlen("<destinationport>")) == 0)
      {
         StoreShort(line + strlen("<destinationport>"), &config[*numConnections].destinationPort);
      }
      else if (strncmp("<a429_parity>", line, strlen("<a429_parity>")) == 0)
      {
         index = strlen("<a429_parity>");
         if (tolower(line[index]) == 'e')
            config[*numConnections].a429Parity = FACE_A429_EVEN;
         else
            config[*numConnections].a429Parity = FACE_A429_ODD;
      }
      else if (strncmp("<a429_speed>", line, strlen("<a429_speed>")) == 0)
      {
         index = strlen("<a429_speed>");
         if (tolower(line[index]) == 'l')
            config[*numConnections].a429Speed = FACE_A429_LOW;
         else
            config[*numConnections].a429Speed = FACE_A429_HIGH;
      }


      // Etc.  Haven't done sourceport, sourceaddress. Not sure of others will be needed
   
   }

   ++*numConnections;   // Increment so it is the actual number of connections versus where the data will be stored

   return true;
}
   

static void StoreString (const char * src, char * dest)
{
   // Read and store until hit the < character
   // Assumes dest has enough room otherwise this will mess up
   while (*src != '<')
      *dest++ = *src++;
}

static void StoreLongLong(const char * src, uint64_t * dest)
{
   *dest = 0;

   // Read and store until hit the < character
   // Assumes only digits, otherwise this will mess up.
   while (*src != '<')
   {
      *dest = (*dest * 10) + (*src - '0');
      ++src;
   }
}

static void StoreShort(const char * src, uint16_t * dest)
{
   *dest = 0;

   // Read and store until hit the < character
   // Assumes only digits, otherwise this will mess up.
   while (*src != '<')
   {
      *dest = (*dest * 10) + (*src - '0');
      ++src;
   }
}

static void StoreByte(const char * src, uint8_t * dest)
{
   *dest = 0;

   // Read and store until hit the < character
   // Assumes only digits, otherwise this will mess up.
   while (*src != '<')
   {
      *dest = (*dest * 10) + (*src - '0');
      ++src;
   }
}
