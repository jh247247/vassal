#include "usart.h"
#include "lcd_control.h"
#include "string.h"
#include "json.h"

#include <jsmn.h>

#define BREAK_CHAR '\r'
#define END_CHAR '\0'

#define JSON_TOKEN_AMOUNT 128 // cant really have more than 128 kinds of
                              // elements on the screen right?
#define JSON_ANIM_TOKEN_AMOUNT 8

// double buffering is kinda hardcoded.
char g_jsonInBuf[2][4096];
unsigned int g_jsonLen[2];
jsmntok_t g_tokens[JSON_TOKEN_AMOUNT];
jsmntok_t *g_animTok[JSON_ANIM_TOKEN_AMOUNT];

int g_jsonFlags;

#define JSON_SETRENDERFLAG (g_jsonFlags |= 0x02)
#define JSON_CLEARRENDERFLAG do{g_jsonFlags &= ~0x02;\
    JSON_SETSKIPRENDERFLAG;} while(0)

// anim flag is only set when we are playing some sort of animation.
#define JSON_SETANIMFLAG (g_jsonFlags |= 0x04)
#define JSON_CLEARANIMFLAG (g_jsonFlags &= ~0x04)




void JSON_init() {
  g_jsonLen[JSON_WRITEBUF] = 0;
  g_jsonLen[JSON_READBUF] = 0;
  g_jsonFlags = 0;
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

// ptr is pointer to start of string to parse (ffw to break char)
// args is arg list to return (gets written to with first 'expected'
// amount of args)
// expected is expected amount of args

// note that in the case of too many args, we stop writing to args and
// ffw to the break char

// returns amount of args found
int getAmountOfArgs(const char** ptr, const char** args, int expected)
{
  int i = 0;
  do {
    if(**ptr == ',' && i != expected) {
      *(args+i) = (*ptr-1);
      i++;
    }
    (*ptr)++;
  } while(**ptr != BREAK_CHAR && **ptr != END_CHAR);
  return i;
}

// Draw the line described by the given string
// args are x0, x1, y0, y1, color
const char* JSON_evalLine(const char* ptr) {
  // should be endptrs for the atoi func.
  const char* data[4] = {NULL};
  if(getAmountOfArgs(&ptr, data, 4) != 4) {
    // how do I reconver? do all the other elements die?
    return ptr; // error! too few args
  }
  LCD_DrawLine(atoi(data[0]),
               atoi(data[1]),
               atoi(data[2]),
               atoi(data[3]),
               atoi(ptr-1));
  return ptr;
}

// args are x0, x1, y0, y1, color
const char* JSON_evalRect(const char* ptr) {
  const char* data[4] = {NULL};
  if(getAmountOfArgs(&ptr, data, 4) != 4) {
    // how do I recover? do all the other elements die?
    return ptr; // error!
  }

  LCD_DrawRect(atoi(data[0]),
               atoi(data[2]),
               atoi(data[1]),
               atoi(data[3]),
               atoi(ptr-1));
  return ptr;
}

// args are x0, x1, y0, y1, color
const char* JSON_evalFilledRect(const char* ptr) {
  const char* data[4] = {NULL};
  if(getAmountOfArgs(&ptr, data, 4) != 4) {
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


// args are x, y, background_color,
// foreground_color, transparent_flag, string
const char* JSON_evalString(const char* ptr) {
  const char* data[5] = {NULL};
  // this one has to be slightly different, hence cannot use the
  // "safe" function
  int i = 0;
  do {
    if(*ptr == ',') {
      data[i] = (ptr-1);
      i++;
    }
    ptr++;
  } while(i != 5 && *ptr != BREAK_CHAR && *ptr != END_CHAR);

  LCD_DrawString(atoi(data[1]),
                 atoi(data[0]),
                 atoi(data[2]),
                 atoi(data[3]),
                 atoi(data[4]),
                 ptr);
  return ptr;
}

const char* JSON_evalClear(const char* ptr) {
  do {
    ptr++;
  } while(*ptr != '\"');

  // only refresh the last one, since clearing multiple times is kinda
  // useless.
  // optimization idea: ffw to a clear since things before don't matter.
  LCD_Clear(atoi(ptr-1));
  return ptr;
}

const char* JSON_renderStatic(int toknum);



// TODO: add macros so that animations can be stored.
int JSON_render() {
  int i;
  int r = 0;
  // parse the input
  jsmn_parser p;
  jsmntok_t* t = g_tokens;
  int len = g_jsonLen[JSON_READBUF];

  if(JSON_GETSKIPRENDERFLAG) {
    // save time by not re-rendering when we don't need to.
    return 0;
  }

  JSON_SETRENDERFLAG;

  // don't need to re-parse if we are playing some animation
  if(!JSON_GETANIMFLAG) {
    jsmn_init(&p);
    r = jsmn_parse(&p, g_jsonInBuf[JSON_READBUF], len, g_tokens,
                     sizeof(g_tokens)/sizeof(g_tokens[0]));
  }

  if (r < 0) {
    // invalid, do not render
    return r;
  }

  // not enough tokens, idk. quit.
  if (r < 1){
    return 1;
  }

  /* Assume the top-level element is an object */
  if(t[0].type != JSMN_OBJECT) {
    return 2;
  }

  // note that all the static stuff is rendered every single time an
  // animation is played
  for(i = 1; i < r; i++) {
    if(t[i].type == JSMN_STRING) {
      JSON_renderStatic(i);
    } else if(t[i].type == JSMN_OBJECT) {
      // woah, we have an object!
      // that means we have to fire up the animation stuff now...
      JSON_SETANIMFLAG;

      // do anim stuff
    }
  }
  // if we aren't doing an animation, we can safely clear the render flag.
  if(!JSON_GETANIMFLAG) {
    JSON_CLEARRENDERFLAG;
  }

  return 0;
}

const char* JSON_renderStatic(int toknum) {
  // only need to check first char of type to determine type
  // this means that you can ensure the drawing order of things on
  // the screen. Handy.
  const char* ptr = g_jsonInBuf[JSON_READBUF]+g_tokens[toknum+1].start;

  if(g_tokens[toknum+1].type != JSMN_STRING){
    // can't really do anything, I don't know how to interpret this
    return ptr;
  }

  char c = *(g_jsonInBuf[JSON_READBUF]+g_tokens[toknum].start);


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

  case 's': // small font string
    do{
      ptr = JSON_evalString(ptr);
    } while(*(ptr+1) != '\"');
    break;

  case 'b': // b -> bitmap base64 encoding (1bpp)
    break;
  case 'c': // c -> color bitmap (16bpp)
    break;
  case 'C': // clear screen
    ptr = JSON_evalClear(ptr);
  default:
    break;
  }
  return ptr;
}
