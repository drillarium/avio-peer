#include "peer.h"

std::string query_get_value(const char *_query, const char *_key)
{
  std::string query(_query);
  std::string key(_key);
  int index = query.find(key);
  if(index == std::string::npos)
    return "";
  index += key.length();
  if(index + key.length() >= query.length() - 2)
    return "";
  if(query[index] != '=')
    return "";
  int end = query.find("&", index, -1);
  
  return query.substr(index+1, end);
}

Peer::Peer()
{
  // "up" event
  m_rest.on("up", [&] () {
    m_bonjour.publish(m_serviceName.c_str(), "_http._tcp", m_rest.port());
  });

  // "error" event
  m_rest.on("error", [&] (const char *error) {
    for(auto e: m_error)
      e(error);
  });

  // "log" event
  m_rest.on("log", [&](const void *message, size_t len) {
    for(auto e : m_log)
      e(message, len);
  });

  // "up" event
  m_bonjour.on("up", [&] () {
    for(auto e : m_up)
      e();
  });

  // "error" event
  m_bonjour.on("error", [&] (const char *error) {
    for(auto e : m_error)
      e(error);
  });
}

void Peer::on(const char *action, const REST_CALLBACK_UP &up)
{
  if(!strcmp(action, "up"))
    m_up.push_back(up);
}

void Peer::on(const char *action, const REST_CALLBACK_ERROR &error)
{
  if(!strcmp(action, "error"))
    m_error.push_back(error);
}

void Peer::on(const char *action, const REST_CALLBACK_LOG &log)
{
  if(!strcmp(action, "log"))
    m_log.push_back(log);
}

bool Peer::start(const char *serviceName, const char *address)
{
  m_serviceName = serviceName;
  return m_rest.start(address);
}
