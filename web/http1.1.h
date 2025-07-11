#pragma once
#include "../core/ht.h"
#include "../core/str.h"
// 
typedef enum HTTPMethod : char {
  HTTP_GET,
  HTTP_HEAD,
  HTTP_POST,
  HTTP_PUT,
  HTTP_DELETE,
  HTTP_CONNECT,
  HTTP_OPTIONS,
  HTTP_TRACE
}HTTPMethod;
typedef enum HTTPRequestTargetForm : char {
  HTTP_REQUEST_TARGET_FORM_ORIGIN,
  HTTP_REQUEST_TARGET_FORM_ABSOLUTE,
  HTTP_REQUEST_TARGET_FORM_AUTHORITY,
  HTTP_REQUEST_TARGET_FORM_ASTERISK
}HTTPRequestTargetForm;
typedef enum HTTPStatusCode : short {
  //Informational 1xx
  Continue = 100, 
  Switching_Protocols = 101, 
  //Successful 2xx
  OK = 200, 
  Created = 201, 
  Accepted = 202, 
  Non_Authoritative_Information = 203, 
  No_Content = 204, 
  Reset_Content = 205, 
  Partial_Content = 206, 
  //Redirection 3xx
  Multiple_Choices = 300, 
  Moved_Permanently = 301, 
  Found = 302, 
  See_Other = 303, 
  Not_Modified = 304, 
  Use_Proxy = 305, 
  Unused_306 = 306, 
  Temporary_Redirect = 307, 
  Permanent_Redirect = 308, 
  //Client Error 4xx
  Bad_Request = 400, 
  Unauthorized = 401, 
  Payment_Required = 402, 
  Forbidden = 403, 
  Not_Found = 404, 
  Method_Not_Allowed = 405, 
  Not_Acceptable = 406, 
  Proxy_Authentication_Required = 407, 
  Request_Timeout = 408, 
  Conflict = 409, 
  Gone = 410, 
  Length_Required = 411, 
  Precondition_Failed = 412, 
  Content_Too_Large = 413, 
  URI_Too_Long = 414, 
  Unsupported_Media_Type = 415, 
  Range_Not_Satisfiable = 416, 
  Expectation_Failed = 417, 
  Unused_418 = 418, 
  Misdirected_Request = 421, 
  Unprocessable_Content = 422, 
  Upgrade_Required = 426, 
  //Server Error 5xx
  Internal_Server_Error = 500, 
  Not_Implemented = 501, 
  Bad_Gateway = 502, 
  Service_Unavailable = 503, 
  Gateway_Timeout = 504, 
  HTTP_Version_Not_Supported = 505 
}HTTPStatusCode;
static const int HTTPValidStatusCodes[] = (int[]){ 

  100, 101, 
  200, 201, 202, 203, 204, 205, 206, 
  300, 301, 302, 303, 304, 305, 306, 307, 308, 
  400, 401, 402, 403, 404, 405, 406, 407, 408, 409, 410, 
  411, 412, 413, 414, 415, 416, 417, 418, 421, 422, 426, 
  500, 501, 502, 503, 504, 505 
};

typedef enum HTTPMessageType : char {
  HTTP_REQUEST,
  HTTP_RESPONSE
}HTTPMessageType;

define_simple_ht(vstr, vstr, HTTPFieldLine_ht);
typedef struct{
  HTTPMessageType type; 
  union{
    struct{
      HTTPMethod method;
      HTTPRequestTargetForm form;
      vstr target;
    }request;
    struct{
      HTTPStatusCode status;
      vstr reason;
    }response;
  };
  struct{
    unsigned char major;
    unsigned char minor;
  }version;
  HTTPFieldLine_ht fields;
  vstr body;
}HTTPMessage;

bool http_parse(vstr raw_message, HTTPMessage *msg);
