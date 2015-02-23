#ifndef JSON_H
#define JSON_H

#define BREAK_CHAR '\r'
#define END_CHAR '\0'


void JSON_init();
int JSON_render();
void JSON_appendToBuf(char c);


#endif /* JSON_H */
