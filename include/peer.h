#pragma once

#include "rest.h"
#include "bonjour.h"

typedef std::function<void()> PEER_CALLBACK_UP;
typedef std::function<void(const char *)> PEER_CALLBACK_ERROR;
typedef std::function<void(const void *, size_t)> REST_CALLBACK_LOG;

std::string query_get_value(const char *query, const char *key);

/*
 *
 */
class Peer
{
public:
  Peer();

  bool start(const char *serviceName, const char *address);
  bool stop() { return m_rest.stop(); }
  bool use(const RESTRouter &router) { return m_rest.use(router); }
  bool use(const std::list<RESTRouter> &router) { return m_rest.use(router); }
  unsigned short port() { return m_rest.port(); }
  void on(const char *action, const PEER_CALLBACK_UP &up);
  void on(const char *action, const PEER_CALLBACK_ERROR &error);
  void on(const char *action, const REST_CALLBACK_LOG &log);
  bool broadcast(const char *buf, size_t len) { return m_rest.broadcast(buf, len); }

protected:
  REST m_rest;
  Bonjour m_bonjour;
  std::list<PEER_CALLBACK_UP> m_up;
  std::list<PEER_CALLBACK_ERROR> m_error;
  std::list<REST_CALLBACK_LOG> m_log;
  std::string m_serviceName;
};
