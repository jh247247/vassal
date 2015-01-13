
#include "lcd_control.h"
#include "string.h"

#include <jsmn.h>


// getting pretty close to the max ram with just these...
// make this 16384 for the final export. no alloc.
char g_jsonInBuf[1000] =
  "{\"l\": \"0,100,0,100,65535\n"
  "0,240,100,100,1000\n"
  "240,140,0,100,10000\n\"}"
  "\"s\": \"200,100,65535,0,0,test\n\""
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



// TODO: make this safer for more args
// Draw the line described by the given string
// args are x0, x1, y0, y1, color
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
  if(i != 4) {
    // how do I recover? do all the other elements die?
    return ptr; // error! too few args
  } // todo: too many args
  LCD_DrawLine(atoi(data[0]),
               atoi(data[1]),
               atoi(data[2]),
               atoi(data[3]),
               atoi(ptr-1));
  return ptr;
}

// TODO: make this safer for more args
// args are x0, x1, y0, y1, color
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
  if(i != 4) {
    // how do I recover? do all the other elements die?
    return ptr; // error!
  } // todo: too many ags
  LCD_DrawRect(atoi(data[0]),
               atoi(data[2]),
               atoi(data[1]),
               atoi(data[3]),
               atoi(ptr-1));
  return ptr;
}


// TODO: make this safer for more args
// args are x0, x1, y0, y1, color
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
  if(i != 4) {
    // how do I recover? do all the other elements die?
    return ptr; // error!
  } // todo: too many args
  LCD_FillRect(atoi(data[0]),
               atoi(data[2]),
               atoi(data[1]),
               atoi(data[3]),
               atoi(ptr-1));
  return ptr;
}

// TODO: make this safer for more args
// args are x, y, background_color,
// foreground_color, transparent_flag, string
const char* JSON_evalString(const char* ptr) {
  const char* data[5] = {NULL};
  int i = 0;
  do {
    if(*ptr == ',') {
      data[i] = (ptr-1);
      i++;
    }
    ptr++;
  } while(i != 5 && *ptr != '\n');


  LCD_DrawString(atoi(data[1]),
                 atoi(data[0]),
                 atoi(data[2]),
                 atoi(data[3]),
                 atoi(data[4]),
                 ptr);
  return ptr;
}

void JSON_render() {
  char c;
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

      c = *(g_jsonInBuf+g_tokens[i].start);

      // only need to check first char of type to determine type
      // this means that you can ensure the drawing order of things on
      // the screen. Handy.
      ptr = g_jsonInBuf+g_tokens[i+1].start;
      switch(c) {
      case 'l':
        do{
          ptr = JSON_evalLine(ptr);
        } while(*(ptr+1) != '\"');
        break;

      case 'r':
        do{
          ptr = JSON_evalRect(ptr);
        } while(*(ptr+1) != '\"');
        break;

      case 'f':
        do{
          ptr = JSON_evalFilledRect(ptr);
        } while(*(ptr+1) != '\"');
        break;

      case 's':
        do{
          ptr = JSON_evalString(ptr);
        } while(*(ptr+1) != '\"');
        break;
      case 'b':
        break;
      case 'c':
        break;
      default:
        break;
      }
      // b -> bitmap base64 encoding
      // c -> color bitmap

    }
  }
}
