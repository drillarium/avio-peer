#include "rest.h"
#include "mongoose.h"

REST::REST()
{

}

bool REST::start(const char *address)
{
  m_address = address;
  m_running = true;
  m_worker = std::thread([&] { this->worker(); });
  return true;
}

bool REST::stop()
{
  if(m_running)
  {
    m_running = false;
    m_worker.join();
  }
  return true;
}

static void lc(const void *buffer, size_t len, void *priv)
{
  REST *rest = static_cast<REST *> (priv);
  for(auto e: rest->m_log)
    e(buffer, len);
}

void REST::worker()
{
  struct mg_mgr mgr;                                                            // Event manager
  mg_log_set("3");                                                              // Set to 3 to enable debug
  mg_log_set_callback(lc, this);
  mg_mgr_init(&mgr);                                                            // Initialise event manager
  auto connection = mg_http_listen(&mgr, m_address.c_str(), REST::fn, this);    // Create HTTP listener
  if(!connection)
  {
    // notify
    for(auto e : m_error)
      e("mg_http_listen listen error");
  }
  else
  {
    // port
    m_port = htons(connection->loc.port);

    // notify
    for(auto e : m_up)
      e();

    // mgr
    m_mgr = &mgr;

    while(m_running)
      mg_mgr_poll(&mgr, 1000);                                                    // Infinite event loop
  }

  m_connMutex.lock();
  m_mgr = NULL;
  mg_mgr_free(&mgr);
  m_connMutex.unlock();
 
 m_running = false;
}

void REST::fn(struct mg_connection *c, int ev, void *ev_data, void *fn_data)
{
  if(ev == MG_EV_HTTP_MSG)
  {
    struct mg_http_message *hm = (struct mg_http_message *) ev_data;
    if(mg_http_match_uri(hm, "/websocket"))
    {
      // Upgrade to websocket. From now on, a connection is a full-duplex
      // Websocket connection, which will receive MG_EV_WS_MSG events.
      mg_ws_upgrade(c, hm, NULL);
    }
    else
    {
      REST *rest = static_cast<REST *> (fn_data);
      rest->m_routersMutex.lock();
      for(auto e : rest->m_routers)
      {
        if(mg_match(hm->method, mg_str(e.method.c_str()), NULL) && mg_http_match_uri(hm, e.api.c_str()))
        {
          // request
          RESTRequest request;
          request.uri = std::string(hm->uri.ptr, hm->uri.len);
          request.body = std::string(hm->body.ptr, hm->body.len);
          request.method = std::string(hm->method.ptr, hm->method.len);
          request.query = std::string(hm->query.ptr, hm->query.len);
          // reply
          RESTReply reply;
          e.callback(request, reply);
          // callback
          mg_http_reply(c, reply.statusCode, reply.headers.c_str(), reply.message.c_str());
        }
      }
      rest->m_routersMutex.unlock();
    }
  }
  else if(ev == MG_EV_WS_MSG)
  {
    // Got websocket frame. Received data is wm->data
    struct mg_ws_message *wm = (struct mg_ws_message *) ev_data;
  }
}

bool REST::use(const RESTRouter &router)
{
  m_routersMutex.lock();
  m_routers.push_back(router);
  m_routersMutex.unlock();

  return true;
}

bool REST::use(const std::list<RESTRouter> &router)
{
  m_routersMutex.lock();
  for(auto e: router)
    m_routers.push_back(e);
  m_routersMutex.unlock();

  return true;
}

void REST::on(const char *action, const REST_CALLBACK_UP &up)
{
  if(!strcmp(action, "up"))
    m_up.push_back(up);
}

void REST::on(const char *action, const REST_CALLBACK_ERROR &error)
{
  if(!strcmp(action, "error"))
    m_error.push_back(error);
}

void REST::on(const char *action, const REST_CALLBACK_LOG &log)
{
  if(!strcmp(action, "log"))
    m_log.push_back(log);
}

bool REST::broadcast(const char *buf, size_t len)
{
  if(!m_mgr)
    return false;

  m_connMutex.lock();
  mg_connection *c = m_mgr->conns;
  while(c)
  {
    if(c->is_websocket)
      mg_ws_send(c, buf, len, WEBSOCKET_OP_BINARY);
    c = c->next;
  }
  m_connMutex.unlock();

  return true;
}