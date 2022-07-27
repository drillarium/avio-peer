#include <iostream>
#include "peer.h"

// peer object
Peer peer;

const RESTRouter router1 = { "GET", "/api/f1", [] (const RESTRequest &request, RESTReply &reply) {
  // websocket test
  peer.broadcast("websocket", 9);       

  reply.message = "{\"result\": 123}";
  reply.statusCode = 200;
}};

const RESTRouter router2 = { "GET", "/api/f2/*", [] (const RESTRequest &request, RESTReply &reply) {
  // websocket test
  peer.broadcast("websocket", 9);

  std::string value = query_get_value(request.query.c_str(), "param1");

  reply.message = "{\"result\": 321}";
  reply.statusCode = 200;
}};

class DummyRouter
{
public:
  DummyRouter()
  {

  }

  std::list<RESTRouter> routes()
  {
    std::list<RESTRouter> routes;
    routes.push_back({ "GET", "/api/dummy1", [&] (const RESTRequest &request, RESTReply &reply) {
      reply.message = "{\"result\": \"dummy1\"}";
      reply.statusCode = 200;
    }});

    routes.push_back({ "GET", "/api/dummy2", [&] (const RESTRequest &request, RESTReply &reply) {
      reply.message = "{\"result\": \"dummy2\"}";
      reply.statusCode = 200;    
    }});

    return routes;
  }
} router;

/*
 *
 */
int main(int argv, char * argc[])
{
  // route
  peer.use(router1);
  peer.use(router2);
  peer.use(router.routes());
  
  // "up" event
  peer.on("up", [] () {
    printf("up PORT: %d\n", peer.port());
  });
  
  // "error" event
  peer.on("error", [] (const char *error) {
    printf("error %s\n", error);
  });

  // "log" event
  peer.on("log", [] (const void *buffer, size_t len) {
    printf("%.*s", (int) len, (const char *) buffer);
  });
  
  // start
  peer.start("helloworld", "http://0.0.0.0:8000");
  
  // Wait
  int input;
  std::cout << ">>";
  std::cin >> input;

  // stop
  peer.stop();

  return 0;
}
