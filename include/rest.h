#pragma once

#include <list>
#include <thread>
#include <mutex>

typedef std::function<void()> REST_CALLBACK_UP;
typedef std::function<void(const char *)> REST_CALLBACK_ERROR;
typedef std::function<void(const void *, size_t)> REST_CALLBACK_LOG;

struct RESTRequest
{
  std::string uri;
  std::string method;
  std::string body;
  std::string query;
};

struct RESTReply
{
  int statusCode;
  std::string headers;
  std::string message;
};

typedef std::function<void(const RESTRequest &request, RESTReply &reply)> REST_CALLBACK;

struct RESTRouter
{
  std::string method;
  std::string api;
  REST_CALLBACK callback;
};

/*
 *
 */
class REST
{
public:
  REST();

  bool start(const char *address);
  bool stop();
  bool use(const RESTRouter &router);
  bool use(const std::list<RESTRouter> &router);
  unsigned short port() { return m_port; }
  void on(const char *action, const REST_CALLBACK_UP &up);
  void on(const char *action, const REST_CALLBACK_ERROR &error);
  void on(const char *action, const REST_CALLBACK_LOG &log);
  bool broadcast(const char *buf, size_t len);

protected:
  void worker();
  static void fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data);

protected:
  std::thread m_worker;
  bool m_running = false;
  std::string m_address;
  std::list<RESTRouter> m_routers;
  std::recursive_mutex m_routersMutex;
  unsigned short m_port = 0;
  std::list<REST_CALLBACK_UP> m_up;
  std::list<REST_CALLBACK_ERROR> m_error;
  struct mg_mgr *m_mgr = NULL;
  std::recursive_mutex m_connMutex;

public:
  std::list<REST_CALLBACK_LOG> m_log;
};
