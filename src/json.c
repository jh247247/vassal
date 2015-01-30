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

#define JSON_AMOUNT_OF_BUFS 2
#define JSON_BUF_LEN 2048

char g_jsonInBuf[JSON_AMOUNT_OF_BUFS][JSON_BUF_LEN];
unsigned int g_jsonLen[JSON_AMOUNT_OF_BUFS];

jsmntok_t g_tokens[JSON_TOKEN_AMOUNT];
jsmntok_t *g_animTok[JSON_ANIM_TOKEN_AMOUNT]; // ptr to tokens holding
					      // animations
int g_animTokLen;

// this is flag stuff

typedef struct {
  int writeBuf;
  int readBuf;
  int readyBufs; // bitfield of bufs ready to be rendered
  int animLock : 1; // lock current buffer since there is animations playing
} json_flags_t;

json_flags_t g_jsonFlags;

#define JSON_ANIM_LOCK_GET (g_jsonFlags.animLock)
#define JSON_ANIM_LOCK_SET (g_jsonFlags.animLock = 1)
#define JSON_ANIM_LOCK_CLEAR do{(g_jsonFlags.animLock = 0);     \
    g_jsonFlags.readBuf = -1;}

#define JSON_BUF_IS_READY(y) (g_jsonFlags.readyBufs & (1<<y))
#define JSON_BUF_SET_READY(y) (g_jsonFlags.readyBufs |= (1<<y))
// can't clear ready flag if locked by animation
#define JSON_BUF_CLEAR_READY(y) do{if(JSON_ANIM_LOCK_GET){break;}       \
    (g_jsonFlags.readyBufs &= ~(1<<y));                                 \
    g_jsonLen[y] = 0;                                                   \
    g_jsonFlags.readBuf = -1;}while(0);


// these should return pretty quickly unless there is super high load
int JSON_nextFullBuf() {
  // linear search for next buffer that is full
  int i = 0;

  // playing animation. can't change buffer.
  if(JSON_ANIM_LOCK_GET) {
    return g_jsonFlags.readBuf;
  }

  while(!JSON_BUF_IS_READY(i) && i < JSON_AMOUNT_OF_BUFS) {
    i++;
  }

  if(i == JSON_AMOUNT_OF_BUFS) {
    return -1; // NO FULL BUFS
  }

  return i;
}

int JSON_nextEmptyBuf() {
  // linear search for next buffer that is empty
  int i = 0;
  while(JSON_BUF_IS_READY(i) && i < JSON_AMOUNT_OF_BUFS) {
    i++;
  }

  if(i == JSON_AMOUNT_OF_BUFS) {
    return -1; // NO EMPTY BUFS
  }

  return i;
}

// this is normal json stuff

void JSON_init() {
  int i;
  for (i = 0; i < JSON_AMOUNT_OF_BUFS; i++) {
    JSON_BUF_CLEAR_READY(i);
  }

  g_animTokLen = 0;
  g_jsonFlags.writeBuf = JSON_nextEmptyBuf();
}


void JSON_appendToBuf(char c) {
  // make sure that the buffer we are writing to is not full right now
  if(g_jsonFlags.writeBuf < 0 || JSON_BUF_IS_READY(g_jsonFlags.writeBuf)) {
    // oops, need to swap bufs
    g_jsonFlags.writeBuf = JSON_nextEmptyBuf();
    if(g_jsonFlags.writeBuf < 0) {
      return; // we can't do anything, no empty bufs
    }
  }

  int i = g_jsonFlags.writeBuf;
  if(g_jsonInBuf[i][0] != '{') {
    g_jsonLen[i] = 0;
  }

  // write and increment the length
  *(g_jsonInBuf[i]+g_jsonLen[i]) = c;
  g_jsonLen[i]++;

  if(*(g_jsonInBuf[i]+g_jsonLen[i]-1) == '\0') {
    // end of string, set the bit int the readybufs
    JSON_BUF_SET_READY(g_jsonFlags.writeBuf);
  }
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
  } while(**ptr != BREAK_CHAR && **ptr != END_CHAR && **ptr != '\"');
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
  }
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

const char* JSON_renderStatic(int toknum, int buf);



// TODO: add macros so that animations can be stored.
int JSON_render() {
  int i;
  int r = 0;
  // parse the input
  int buf = JSON_nextFullBuf(); // wait, what do I do with
                                // animations... FIXME

  if(buf < 0) {
    return 3; // wooo
  }

  jsmn_parser p;
  jsmntok_t* t = g_tokens;
  int len = g_jsonLen[buf];

  // don't need to re-parse if we are playing some animation
  if(!JSON_ANIM_LOCK_GET) {
    jsmn_init(&p);
    r = jsmn_parse(&p, g_jsonInBuf[buf], len, g_tokens,
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
      JSON_renderStatic(i,buf);
    } else if(t[i].type == JSMN_OBJECT) {
      // woah, we have an object!
      // that means we have to fire up the animation stuff now...
      // TODO

      // lock read buffer so that we don't overwrite it
      JSON_ANIM_LOCK_SET;

      // do anim stuff
    }
  }

  // TODO anims
  JSON_BUF_CLEAR_READY(buf);

  return 0;
}

const char* JSON_renderStatic(int toknum, int buf) {
  // only need to check first char of type to determine type
  // this means that you can ensure the drawing order of things on
  // the screen. Handy.
  const char* ptr = g_jsonInBuf[buf]+g_tokens[toknum+1].start;

  if(g_tokens[toknum+1].type != JSMN_STRING){
    // can't really do anything, I don't know how to interpret this
    return ptr;
  }

  char c = *(g_jsonInBuf[buf]+g_tokens[toknum].start);

  switch(c) {
  case 'l':
    do{
      ptr = JSON_evalLine(ptr);
    } while(*(ptr+1) != '\"' && *(ptr) != END_CHAR);
    break;

  case 'r':
    do{
      ptr = JSON_evalRect(ptr);
    } while(*(ptr+1) != '\"' && *(ptr) != END_CHAR);
    break;

  case 'f':
    do{
      ptr = JSON_evalFilledRect(ptr);
    } while(*(ptr+1) != '\"' && *(ptr) != END_CHAR);
    break;

  case 's': // small font string
    do{
      ptr = JSON_evalString(ptr);
    } while(*(ptr+1) != '\"' && *(ptr) != END_CHAR);
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
