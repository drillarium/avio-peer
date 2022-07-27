#include "bonjour.h"

const char * dnsServiceStrError(DNSServiceErrorType err)
{
  switch(err)
  {
    case kDNSServiceErr_NoError: return "kDNSServiceErr_NoError";
    case kDNSServiceErr_Unknown: return "kDNSServiceErr_Unknown";
    case kDNSServiceErr_NoSuchName: return "kDNSServiceErr_NoSuchName";
    case kDNSServiceErr_NoMemory: return "kDNSServiceErr_NoMemory";
    case kDNSServiceErr_BadParam: return "kDNSServiceErr_BadParam";
    case kDNSServiceErr_BadReference: return "kDNSServiceErr_BadReference";
    case kDNSServiceErr_BadState: return "kDNSServiceErr_BadState";
    case kDNSServiceErr_BadFlags: return "kDNSServiceErr_BadFlags";
    case kDNSServiceErr_Unsupported: return "kDNSServiceErr_Unsupported";
    case kDNSServiceErr_NotInitialized: return "kDNSServiceErr_NotInitialized";
    case kDNSServiceErr_AlreadyRegistered: return "kDNSServiceErr_AlreadyRegistered";
    case kDNSServiceErr_NameConflict: return "kDNSServiceErr_NameConflict";
    case kDNSServiceErr_Invalid: return "kDNSServiceErr_Invalid";
    case kDNSServiceErr_Firewall: return "kDNSServiceErr_Firewall";
    case kDNSServiceErr_Incompatible: return "kDNSServiceErr_Incompatible";
    case kDNSServiceErr_BadInterfaceIndex: return "kDNSServiceErr_BadInterfaceIndex";
    case kDNSServiceErr_Refused: return "kDNSServiceErr_Refused";
    case kDNSServiceErr_NoSuchRecord: return "kDNSServiceErr_NoSuchRecord";
    case kDNSServiceErr_NoAuth: return "kDNSServiceErr_NoAuth";
    case kDNSServiceErr_NoSuchKey: return "kDNSServiceErr_NoSuchKey";
    case kDNSServiceErr_NATTraversal: return "kDNSServiceErr_NATTraversal";
    case kDNSServiceErr_DoubleNAT: return "kDNSServiceErr_DoubleNAT";
    case kDNSServiceErr_BadTime: return "kDNSServiceErr_BadTime";
    default:
      static char buf[32];
      snprintf(buf, sizeof(buf), "%d", err);
      return buf;
  }
}

Bonjour::Bonjour()
{

}

bool Bonjour::publish(const char *name, const char *type, unsigned short _port)
{
  uint16_t port = htons(_port);
  DNSServiceErrorType err = DNSServiceRegister(&m_register,
                                               0 /* interface */,
                                               0 /* flags */,
                                               name /* name */,
                                               type /* regtype */,
                                               NULL /* domain */,
                                               NULL /* host */,
                                               port,
                                               0 /* txtLen */,
                                               NULL /* txtRecord */,
                                               Bonjour::registerReplyCallback /* callback */,
                                               this /* context */);

  if(err != kDNSServiceErr_NoError)
    return false;
 
  err = DNSServiceProcessResult(m_register);

  return (err == kDNSServiceErr_NoError);
}

void Bonjour::on(const char *action, const BJ_CALLBACK_UP &up)
{
  if(!strcmp(action, "up"))
    m_up.push_back(up);
}

void Bonjour::on(const char *action, const BJ_CALLBACK_ERROR &error)
{
  if(!strcmp(action, "error"))
    m_error.push_back(error);
}

void Bonjour::registerReplyCallback(DNSServiceRef sdRef, DNSServiceFlags flags, DNSServiceErrorType errorCode, const char *name, const char *regtype, const char *domain, void *context)
{
  Bonjour *bj = static_cast<Bonjour *> (context);
  if(errorCode == kDNSServiceErr_NoError)
  {
    for(auto e : bj->m_up)
      e();
  }
  else
  {
    for(auto e : bj->m_error)
      e(dnsServiceStrError(errorCode));
  }
}
