
#include "lcd_control.h"
#include "string.h"

#include <jsmn.h>


// getting pretty close to the max ram with just these...
// make this 16384 for the final export. no alloc.
char g_jsonInBuf[1000] =
  "{\"l\": \"0,100,0,100,65535\n"
  "0,240,100,100,1000\n"
  "240,140,0,100,10000\n\"}"
  "\"f\": \"100,150,100,150,60000\n\""
  "\"r\": \"100,150,100,150,6000\n\"}\0";

unsigned int g_jsonLen;

jsmn_parser g_parser;
jsmntok_t g_tokens[128]; // cant really have more than 128 kinds of
                         // elements on the screen right?


void JSON_init() {
  jsmn_init(&g_parser);
  g_jsonLen = strlen(g_jsonInBuf);
}

int atoi(const char* endptr) {
  int mult = 1;
  int retval = 0;
  while(*endptr >= '0' && *endptr <= '9') {
    retval += mult*(*endptr-'0');
    endptr--;
    mult *= 10;
  }
  return retval;
}

static int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
  if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start &&
      strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
    return 0;
  }
  return -1;
}


// Draw the line described by the given string
const char* JSON_evalLine(const char* ptr) {
  // should be endptrs for the atoi func.
  const char* data[4] = {NULL};
  int i = 0;
  do {
    if(*ptr == ',') {
      data[i] = (ptr-1);
      i++;
    }
    ptr++;
  } while(*ptr != '\n');
  LCD_DrawLine(atoi(data[0]),
               atoi(data[1]),
               atoi(data[2]),
               atoi(data[3]),
               atoi(ptr-1));
  return ptr;
}

const char* JSON_evalRect(const char* ptr) {
  const char* data[4] = {NULL};
  int i = 0;
  do {
    if(*ptr == ',') {
      data[i] = (ptr-1);
      i++;
    }
    ptr++;
  } while(*ptr != '\n');
  LCD_DrawRect(atoi(data[0]),
               atoi(data[2]),
               atoi(data[1]),
               atoi(data[3]),
               atoi(ptr-1));
  return ptr;
}

const char* JSON_evalFilledRect(const char* ptr) {
  const char* data[4] = {NULL};
  int i = 0;
  do {
    if(*ptr == ',') {
      data[i] = (ptr-1);
      i++;
    }
    ptr++;
  } while(*ptr != '\n');
  LCD_FillRect(atoi(data[0]),
               atoi(data[2]),
               atoi(data[1]),
               atoi(data[3]),
               atoi(ptr-1));
  return ptr;
}



void JSON_render() {
  int i;
  // parse the input
  int r = jsmn_parse(&g_parser, g_jsonInBuf, g_jsonLen, g_tokens,
                     sizeof(g_tokens)/sizeof(g_tokens[0]));
  const char* ptr;

  if (r < 0) {
    return;
  }

  /* Assume the top-level element is an object */
  if (r < 1 || g_tokens[0].type != JSMN_OBJECT) {
    return;
  }

  for(i = 1; i < r; i++) {
    if(g_tokens[i].type == JSMN_STRING) {
      if(g_tokens[i+1].type != JSMN_STRING){
        // iunno.
        continue;
      }
      // only need to check first char of type to determine type
      // this means that you can ensure the drawing order of things on
      // the screen. Handy.
      ptr = g_jsonInBuf+g_tokens[i+1].start;
      if(jsoneq(g_jsonInBuf, &g_tokens[i], "l") == 0){
        do{
          ptr = JSON_evalLine(ptr);
        } while(*(ptr+1) != '\"');
      }

      if(jsoneq(g_jsonInBuf, &g_tokens[i], "r") == 0){
        do{
          ptr = JSON_evalRect(ptr);
        } while(*(ptr+1) != '\"');
      }

      if(jsoneq(g_jsonInBuf, &g_tokens[i], "f") == 0){
        do{
          ptr = JSON_evalFilledRect(ptr);
        } while(*(ptr+1) != '\"');
      }


    }

  }
}
