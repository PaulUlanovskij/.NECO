#include "ht.h"

uint64_t djb2(void* mem, int size){
  uint64_t hash = 5381;

  for(int i = 0; i < size; i++){
    hash = ((hash << 5) + hash) + ((char*)mem)[i]; /* hash * 33 + c */
  }
  
  return hash;
}

uint64_t bool_hash(bool x) {return x;}
uint64_t char_hash(char x) {return x;}
uint64_t short_hash(short x) {return x;}
uint64_t int_hash(int x) {return x;}
uint64_t long_hash(long x) {return x;}
uint64_t float_hash(float x) {return *(int*)&x;}
uint64_t double_hash(double x) {return *(long*)&x;}

uint64_t cstr_hash(cstr str){
  return djb2(str, strlen(str));
} 
uint64_t vstr_hash(vstr str){
  return djb2(str.items, str.length);
}
uint64_t dstr_hash(dstr str){
  return djb2(str.items, str.length-1);
} 
uint64_t cstr_da_hash(cstr_da da){
  uint64_t hash = 0;
  da_foreach(&da, str){
    hash ^= djb2(*str, strlen(*str));
  }
  return hash;
} 
uint64_t vstr_da_hash(vstr_da da){
  uint64_t hash = 0;
  da_foreach(&da, str){
    hash ^= djb2(str->items, str->length);
  }
  return hash;
} 

