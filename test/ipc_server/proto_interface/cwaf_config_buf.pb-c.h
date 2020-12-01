/* Generated by the protocol buffer compiler.  DO NOT EDIT! */
/* Generated from: cwaf_config_buf.proto */

#ifndef PROTOBUF_C_cwaf_5fconfig_5fbuf_2eproto__INCLUDED
#define PROTOBUF_C_cwaf_5fconfig_5fbuf_2eproto__INCLUDED

#include <protobuf-c/protobuf-c.h>

PROTOBUF_C__BEGIN_DECLS

#if PROTOBUF_C_VERSION_NUMBER < 1003000
# error This file was generated by a newer version of protoc-c which is incompatible with your libprotobuf-c headers. Please update your headers.
#elif 1003003 < PROTOBUF_C_MIN_COMPILER_VERSION
# error This file was generated by an older version of protoc-c which is incompatible with your libprotobuf-c headers. Please regenerate this file with a newer version of protoc-c.
#endif


typedef struct _DnsConfig DnsConfig;
typedef struct _StaticRoute StaticRoute;
typedef struct _DefaultGateway DefaultGateway;
typedef struct _ManageService ManageService;
typedef struct _Interface Interface;
typedef struct _MatchOption MatchOption;
typedef struct _DefineRule DefineRule;
typedef struct _SslCertificate SslCertificate;
typedef struct _Bizserver Bizserver;
typedef struct _AttackDetect AttackDetect;
typedef struct _SiteSecurity SiteSecurity;
typedef struct _PredefSignRule PredefSignRule;
typedef struct _VirtualService VirtualService;
typedef struct _CwConfig CwConfig;


/* --- enums --- */


/* --- messages --- */

/*
 *DNS配置
 */
struct  _DnsConfig
{
  ProtobufCMessage base;
  int32_t id;
  char *interface_name;
  char *host_name;
  char *preferred_server;
  char *alternative_server;
};
#define DNS_CONFIG__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&dns_config__descriptor) \
    , 0, (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string }


/*
 *静态路由  
 */
struct  _StaticRoute
{
  ProtobufCMessage base;
  int32_t id;
  char *interface_name;
  char *dst_ip;
  char *subnet_mask;
  char *gateway;
};
#define STATIC_ROUTE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&static_route__descriptor) \
    , 0, (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string }


/*
 *默认网关  
 */
struct  _DefaultGateway
{
  ProtobufCMessage base;
  int32_t id;
  char *interface_name;
  char *ipv4_gateway;
  char *ipv6_gateway;
};
#define DEFAULT_GATEWAY__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&default_gateway__descriptor) \
    , 0, (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string }


/*
 *管理服务  
 */
struct  _ManageService
{
  ProtobufCMessage base;
  int32_t id;
  char *name;
  char *port;
  char *whitelist;
  char *ser_interface;
};
#define MANAGE_SERVICE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&manage_service__descriptor) \
    , 0, (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string }


/*
 *接口配置
 */
struct  _Interface
{
  ProtobufCMessage base;
  int32_t id;
  char *name;
  int32_t type;
  int32_t speed;
  char *ipv4;
  char *ipv6;
  char *mac;
  char *mtu;
  int32_t negotiate;
  int32_t half;
};
#define INTERFACE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&interface__descriptor) \
    , 0, (char *)protobuf_c_empty_string, 0, 0, (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0, 0 }


struct  _MatchOption
{
  ProtobufCMessage base;
  char *field;
  int32_t match_rule;
  char *expression;
};
#define MATCH_OPTION__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&match_option__descriptor) \
    , (char *)protobuf_c_empty_string, 0, (char *)protobuf_c_empty_string }


/*
 *自定义规则  
 */
struct  _DefineRule
{
  ProtobufCMessage base;
  int32_t id;
  int32_t seq;
  int32_t mode;
  int32_t enable;
  int32_t level;
  int32_t log_type;
  char *name;
  size_t n_match_option_list;
  MatchOption **match_option_list;
  char *expire_time;
};
#define DEFINE_RULE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&define_rule__descriptor) \
    , 0, 0, 0, 0, 0, 0, (char *)protobuf_c_empty_string, 0,NULL, (char *)protobuf_c_empty_string }


/*
 *SSL证书  
 */
struct  _SslCertificate
{
  ProtobufCMessage base;
  int32_t id;
  char *cert_name;
  int32_t enable;
  char *cert_path;
  char *key_path;
  char *cert_password;
};
#define SSL_CERTIFICATE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&ssl_certificate__descriptor) \
    , 0, (char *)protobuf_c_empty_string, 0, (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string }


/*
 *业务服务器  
 */
struct  _Bizserver
{
  ProtobufCMessage base;
  char *protocol;
  char *address;
  char *port;
};
#define BIZSERVER__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&bizserver__descriptor) \
    , (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string }


/*
 *攻击检测模块
 */
struct  _AttackDetect
{
  ProtobufCMessage base;
  char *name;
  int32_t enable;
  int32_t block;
  int32_t log;
};
#define ATTACK_DETECT__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&attack_detect__descriptor) \
    , (char *)protobuf_c_empty_string, 0, 0, 0 }


/*
 *站点安全  
 */
struct  _SiteSecurity
{
  ProtobufCMessage base;
  int32_t id;
  char *name;
  char *decode_type;
  int32_t block_type;
  size_t n_attack_detect_list;
  AttackDetect **attack_detect_list;
  char *http_req_body_detect;
};
#define SITE_SECURITY__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&site_security__descriptor) \
    , 0, (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0, 0,NULL, (char *)protobuf_c_empty_string }


/*
 *特征库升级
 */
struct  _PredefSignRule
{
  ProtobufCMessage base;
  int32_t action;
  char *predef_sign_rule_path;
};
#define PREDEF_SIGN_RULE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&predef_sign_rule__descriptor) \
    , 0, (char *)protobuf_c_empty_string }


/*
 *虚拟服务  
 */
struct  _VirtualService
{
  ProtobufCMessage base;
  int32_t id;
  char *name;
  char *bind_domain;
  int32_t enable;
  int32_t ssl_enable;
  char *ports;
  size_t n_cw_ssl_certificate;
  SslCertificate **cw_ssl_certificate;
  char *ssl_version;
  char *ssl_cipher;
  int32_t response_type;
  size_t n_bizserver_list;
  Bizserver **bizserver_list;
  size_t n_cw_site_security;
  SiteSecurity **cw_site_security;
};
#define VIRTUAL_SERVICE__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&virtual_service__descriptor) \
    , 0, (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0, 0, (char *)protobuf_c_empty_string, 0,NULL, (char *)protobuf_c_empty_string, (char *)protobuf_c_empty_string, 0, 0,NULL, 0,NULL }


struct  _CwConfig
{
  ProtobufCMessage base;
  int32_t action;
  DnsConfig *dnsconfig;
  ManageService *manageservice;
  DefaultGateway *defaultgateway;
  StaticRoute *staticroute;
  Interface *interface;
  DefineRule *definerule;
  SslCertificate *sslcertificate;
  SiteSecurity *sitesecurity;
  PredefSignRule *predefsignrule;
  VirtualService *virtualservice;
};
#define CW_CONFIG__INIT \
 { PROTOBUF_C_MESSAGE_INIT (&cw_config__descriptor) \
    , 0, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL }


/* DnsConfig methods */
void   dns_config__init
                     (DnsConfig         *message);
size_t dns_config__get_packed_size
                     (const DnsConfig   *message);
size_t dns_config__pack
                     (const DnsConfig   *message,
                      uint8_t             *out);
size_t dns_config__pack_to_buffer
                     (const DnsConfig   *message,
                      ProtobufCBuffer     *buffer);
DnsConfig *
       dns_config__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   dns_config__free_unpacked
                     (DnsConfig *message,
                      ProtobufCAllocator *allocator);
/* StaticRoute methods */
void   static_route__init
                     (StaticRoute         *message);
size_t static_route__get_packed_size
                     (const StaticRoute   *message);
size_t static_route__pack
                     (const StaticRoute   *message,
                      uint8_t             *out);
size_t static_route__pack_to_buffer
                     (const StaticRoute   *message,
                      ProtobufCBuffer     *buffer);
StaticRoute *
       static_route__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   static_route__free_unpacked
                     (StaticRoute *message,
                      ProtobufCAllocator *allocator);
/* DefaultGateway methods */
void   default_gateway__init
                     (DefaultGateway         *message);
size_t default_gateway__get_packed_size
                     (const DefaultGateway   *message);
size_t default_gateway__pack
                     (const DefaultGateway   *message,
                      uint8_t             *out);
size_t default_gateway__pack_to_buffer
                     (const DefaultGateway   *message,
                      ProtobufCBuffer     *buffer);
DefaultGateway *
       default_gateway__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   default_gateway__free_unpacked
                     (DefaultGateway *message,
                      ProtobufCAllocator *allocator);
/* ManageService methods */
void   manage_service__init
                     (ManageService         *message);
size_t manage_service__get_packed_size
                     (const ManageService   *message);
size_t manage_service__pack
                     (const ManageService   *message,
                      uint8_t             *out);
size_t manage_service__pack_to_buffer
                     (const ManageService   *message,
                      ProtobufCBuffer     *buffer);
ManageService *
       manage_service__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   manage_service__free_unpacked
                     (ManageService *message,
                      ProtobufCAllocator *allocator);
/* Interface methods */
void   interface__init
                     (Interface         *message);
size_t interface__get_packed_size
                     (const Interface   *message);
size_t interface__pack
                     (const Interface   *message,
                      uint8_t             *out);
size_t interface__pack_to_buffer
                     (const Interface   *message,
                      ProtobufCBuffer     *buffer);
Interface *
       interface__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   interface__free_unpacked
                     (Interface *message,
                      ProtobufCAllocator *allocator);
/* MatchOption methods */
void   match_option__init
                     (MatchOption         *message);
size_t match_option__get_packed_size
                     (const MatchOption   *message);
size_t match_option__pack
                     (const MatchOption   *message,
                      uint8_t             *out);
size_t match_option__pack_to_buffer
                     (const MatchOption   *message,
                      ProtobufCBuffer     *buffer);
MatchOption *
       match_option__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   match_option__free_unpacked
                     (MatchOption *message,
                      ProtobufCAllocator *allocator);
/* DefineRule methods */
void   define_rule__init
                     (DefineRule         *message);
size_t define_rule__get_packed_size
                     (const DefineRule   *message);
size_t define_rule__pack
                     (const DefineRule   *message,
                      uint8_t             *out);
size_t define_rule__pack_to_buffer
                     (const DefineRule   *message,
                      ProtobufCBuffer     *buffer);
DefineRule *
       define_rule__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   define_rule__free_unpacked
                     (DefineRule *message,
                      ProtobufCAllocator *allocator);
/* SslCertificate methods */
void   ssl_certificate__init
                     (SslCertificate         *message);
size_t ssl_certificate__get_packed_size
                     (const SslCertificate   *message);
size_t ssl_certificate__pack
                     (const SslCertificate   *message,
                      uint8_t             *out);
size_t ssl_certificate__pack_to_buffer
                     (const SslCertificate   *message,
                      ProtobufCBuffer     *buffer);
SslCertificate *
       ssl_certificate__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   ssl_certificate__free_unpacked
                     (SslCertificate *message,
                      ProtobufCAllocator *allocator);
/* Bizserver methods */
void   bizserver__init
                     (Bizserver         *message);
size_t bizserver__get_packed_size
                     (const Bizserver   *message);
size_t bizserver__pack
                     (const Bizserver   *message,
                      uint8_t             *out);
size_t bizserver__pack_to_buffer
                     (const Bizserver   *message,
                      ProtobufCBuffer     *buffer);
Bizserver *
       bizserver__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   bizserver__free_unpacked
                     (Bizserver *message,
                      ProtobufCAllocator *allocator);
/* AttackDetect methods */
void   attack_detect__init
                     (AttackDetect         *message);
size_t attack_detect__get_packed_size
                     (const AttackDetect   *message);
size_t attack_detect__pack
                     (const AttackDetect   *message,
                      uint8_t             *out);
size_t attack_detect__pack_to_buffer
                     (const AttackDetect   *message,
                      ProtobufCBuffer     *buffer);
AttackDetect *
       attack_detect__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   attack_detect__free_unpacked
                     (AttackDetect *message,
                      ProtobufCAllocator *allocator);
/* SiteSecurity methods */
void   site_security__init
                     (SiteSecurity         *message);
size_t site_security__get_packed_size
                     (const SiteSecurity   *message);
size_t site_security__pack
                     (const SiteSecurity   *message,
                      uint8_t             *out);
size_t site_security__pack_to_buffer
                     (const SiteSecurity   *message,
                      ProtobufCBuffer     *buffer);
SiteSecurity *
       site_security__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   site_security__free_unpacked
                     (SiteSecurity *message,
                      ProtobufCAllocator *allocator);
/* PredefSignRule methods */
void   predef_sign_rule__init
                     (PredefSignRule         *message);
size_t predef_sign_rule__get_packed_size
                     (const PredefSignRule   *message);
size_t predef_sign_rule__pack
                     (const PredefSignRule   *message,
                      uint8_t             *out);
size_t predef_sign_rule__pack_to_buffer
                     (const PredefSignRule   *message,
                      ProtobufCBuffer     *buffer);
PredefSignRule *
       predef_sign_rule__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   predef_sign_rule__free_unpacked
                     (PredefSignRule *message,
                      ProtobufCAllocator *allocator);
/* VirtualService methods */
void   virtual_service__init
                     (VirtualService         *message);
size_t virtual_service__get_packed_size
                     (const VirtualService   *message);
size_t virtual_service__pack
                     (const VirtualService   *message,
                      uint8_t             *out);
size_t virtual_service__pack_to_buffer
                     (const VirtualService   *message,
                      ProtobufCBuffer     *buffer);
VirtualService *
       virtual_service__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   virtual_service__free_unpacked
                     (VirtualService *message,
                      ProtobufCAllocator *allocator);
/* CwConfig methods */
void   cw_config__init
                     (CwConfig         *message);
size_t cw_config__get_packed_size
                     (const CwConfig   *message);
size_t cw_config__pack
                     (const CwConfig   *message,
                      uint8_t             *out);
size_t cw_config__pack_to_buffer
                     (const CwConfig   *message,
                      ProtobufCBuffer     *buffer);
CwConfig *
       cw_config__unpack
                     (ProtobufCAllocator  *allocator,
                      size_t               len,
                      const uint8_t       *data);
void   cw_config__free_unpacked
                     (CwConfig *message,
                      ProtobufCAllocator *allocator);
/* --- per-message closures --- */

typedef void (*DnsConfig_Closure)
                 (const DnsConfig *message,
                  void *closure_data);
typedef void (*StaticRoute_Closure)
                 (const StaticRoute *message,
                  void *closure_data);
typedef void (*DefaultGateway_Closure)
                 (const DefaultGateway *message,
                  void *closure_data);
typedef void (*ManageService_Closure)
                 (const ManageService *message,
                  void *closure_data);
typedef void (*Interface_Closure)
                 (const Interface *message,
                  void *closure_data);
typedef void (*MatchOption_Closure)
                 (const MatchOption *message,
                  void *closure_data);
typedef void (*DefineRule_Closure)
                 (const DefineRule *message,
                  void *closure_data);
typedef void (*SslCertificate_Closure)
                 (const SslCertificate *message,
                  void *closure_data);
typedef void (*Bizserver_Closure)
                 (const Bizserver *message,
                  void *closure_data);
typedef void (*AttackDetect_Closure)
                 (const AttackDetect *message,
                  void *closure_data);
typedef void (*SiteSecurity_Closure)
                 (const SiteSecurity *message,
                  void *closure_data);
typedef void (*PredefSignRule_Closure)
                 (const PredefSignRule *message,
                  void *closure_data);
typedef void (*VirtualService_Closure)
                 (const VirtualService *message,
                  void *closure_data);
typedef void (*CwConfig_Closure)
                 (const CwConfig *message,
                  void *closure_data);

/* --- services --- */


/* --- descriptors --- */

extern const ProtobufCMessageDescriptor dns_config__descriptor;
extern const ProtobufCMessageDescriptor static_route__descriptor;
extern const ProtobufCMessageDescriptor default_gateway__descriptor;
extern const ProtobufCMessageDescriptor manage_service__descriptor;
extern const ProtobufCMessageDescriptor interface__descriptor;
extern const ProtobufCMessageDescriptor match_option__descriptor;
extern const ProtobufCMessageDescriptor define_rule__descriptor;
extern const ProtobufCMessageDescriptor ssl_certificate__descriptor;
extern const ProtobufCMessageDescriptor bizserver__descriptor;
extern const ProtobufCMessageDescriptor attack_detect__descriptor;
extern const ProtobufCMessageDescriptor site_security__descriptor;
extern const ProtobufCMessageDescriptor predef_sign_rule__descriptor;
extern const ProtobufCMessageDescriptor virtual_service__descriptor;
extern const ProtobufCMessageDescriptor cw_config__descriptor;

PROTOBUF_C__END_DECLS


#endif  /* PROTOBUF_C_cwaf_5fconfig_5fbuf_2eproto__INCLUDED */
