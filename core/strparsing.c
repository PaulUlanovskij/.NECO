#include <string.h>
#include <ctype.h>

#include "str.h"

bool cstr_is_int(cstr str){
  return vstr_is_int((vstr){.items = str, .length = strlen(str)}); 
}
bool dstr_is_int(dstr ds){
  return vstr_is_int((vstr){.items = ds.items, .length = ds.length-1}); 
}

bool vstr_is_int(vstr str){
  for(int i = 0; i < str.length; i++){
    char c = str.items[i];
    if(c == '+' || c == '-'){
      if(i!=0){
        return false;
      }
    }else if(isdigit(c) == false){
      return false;
    }
  }
  return true;
}

typedef enum {
  FFMT_UNDEFINED, 
  FFMT_DECIMAL_PRE, 
  FFMT_DECIMAL_MESO, 
  FFMT_DECIMAL_POST,
  FFMT_HEX_PRE, 
  FFMT_HEX_MESO, 
  FFMT_HEX_POST, 
  FFMT_INF, 
  FFMT_NAN
}FloatFmts;

bool cstr_is_float(cstr str){
  return vstr_is_float((vstr){.items = str, .length = strlen(str)}); 
}
bool dstr_is_float(dstr ds){
  return vstr_is_float((vstr){.items = ds.items, .length = ds.length-1}); 
}

bool vstr_is_float(vstr str){
  char prev = '\0';
  FloatFmts ffmt = FFMT_UNDEFINED;
  
  for(int i = 0; i < str.length; i++){
    char c = tolower(str.items[i]);
    switch(ffmt){
      case FFMT_UNDEFINED:{
        if(c == '+' || c == '-'){
          if(prev == '\0'){
            break; 
          }else{
            return false;
          }
        }else if(c == '0'){
          if(isdigit(prev)){
            ffmt = FFMT_DECIMAL_PRE;
          }
          break;
        }else if(c == 'x'){
          if(prev == '0'){
            ffmt = FFMT_HEX_PRE;
            break;
          }else{
            return false;
          }
        }else if(c == 'i'){
          if(prev == '+' || prev == '-' || prev == '\0'){
            ffmt = FFMT_INF;
            break;
          }else{
            return false;            
          }
        }else if(c == 'n'){
          if(prev == '+' || prev == '-' || prev == '\0'){
            ffmt = FFMT_NAN;
            break;
          }else{
            return false;            
          }
        }
      }break;
      case FFMT_DECIMAL_PRE:{
        if(c == '.'){
          ffmt = FFMT_DECIMAL_MESO;
          break;
        }
        if(c == 'e'){
          ffmt = FFMT_DECIMAL_POST;
          break;
        }
        if(isdigit(c) == false){
          return false;            
        }
      }break;
      case FFMT_DECIMAL_MESO:{
        if(c == 'e'){
          ffmt = FFMT_DECIMAL_POST;
          break;
        }
        if(isdigit(c) == false){
          return false;            
        }
      }break;
      case FFMT_DECIMAL_POST:{
        if((c == '+' || c == '-') && prev == 'e'){
          break;
        }
        if(isdigit(c) == false){
          return false;            
        }
      }break;
      case FFMT_HEX_PRE:{
        if(c == '.'){
          ffmt = FFMT_HEX_MESO;
          break;
        }
        if(c == 'p'){
          ffmt = FFMT_HEX_POST;
          break;
        }
        if(isxdigit(c) == false){
          return false;            
        }
      }break;
      case FFMT_HEX_MESO:{
        if(c == 'p'){
          ffmt = FFMT_HEX_POST;
          break;
        }
        if(isxdigit(c) == false){
          return false;            
        }
      }break;
      case FFMT_HEX_POST:{
        if((c == '+' || c == '-') && prev == 'p'){
          break;
        }
        if(isxdigit(c) == false){
          return false;            
        }
      }break;
      case FFMT_INF:{
        bool has_sign = (str.items[0] == '+' || str.items[0] == '-'); 
        int efflen = str.length - (has_sign ? 1 : 0); 
        if(efflen == 3){
          char* short_inf = "inf";
          char* curstr = str.items + (has_sign ? 1 : 0);
          for(; *short_inf == tolower(*curstr) && curstr < str.items + str.length; curstr++, short_inf++);
          if(curstr == str.items + str.length){
            return true;
          }
        }else if(efflen == 8){
          char* short_inf = "infinity";
          char* curstr = str.items + (has_sign ? 1 : 0);
          for(; *short_inf == tolower(*curstr) && curstr < str.items + str.length; curstr++, short_inf++);
          if(curstr == str.items + str.length){
            return true;
          }
        }
        return false;            
      }break;
      case FFMT_NAN:{
        bool has_sign = (str.items[0] == '+' || str.items[0] == '-'); 
        int efflen = str.length - (has_sign ? 1 : 0); 
        if(efflen < 3){
          return false;            
        }
        char* short_inf = "nan";
        char* curstr = str.items + (has_sign ? 1 : 0);
        for(; *short_inf == tolower(*curstr) && curstr < str.items + str.length; curstr++, short_inf++);
        if(curstr == str.items + (has_sign ? 1 : 0) + 3){
          if(efflen == 3){
            return true;
          }
          for(int j = (has_sign ? 1 : 0) + 3; j < str.length; j++){
            c = tolower(str.items[j]);
            if(isalnum(c) == false && c == '_' == false){
              return false;            
            }
          }
          return true;
        }
        return false;            
      }break;
    }  
    prev = c;
  }
  if(ffmt == FFMT_DECIMAL_POST && prev == 'c'){
    return false;            
  }
  if(ffmt == FFMT_HEX_POST && prev == 'p'){
    return false;            
  }

  return true;
}

