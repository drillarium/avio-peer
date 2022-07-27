#pragma once

#include <list>
#include <functional>
#include <dns_sd.h>

typedef std::function<void()> BJ_CALLBACK_UP;
typedef std::function<void(const char *)> BJ_CALLBACK_ERROR;

/*
 *
 */
class Bonjour
{
public:
  Bonjour();

  bool publish(const char *name, const char *type, unsigned short port);
  void on(const char *action, const BJ_CALLBACK_UP &up);
  void on(const char *action, const BJ_CALLBACK_ERROR &error);

protected:  
  static void registerReplyCallback(DNSServiceRef sdRef, DNSServiceFlags flags, DNSServiceErrorType errorCode, const char *name, const char *regtype, const char *domain, void *context);

protected:
  DNSServiceRef m_register;
  std::list<BJ_CALLBACK_UP> m_up;
  std::list<BJ_CALLBACK_ERROR> m_error;
};
