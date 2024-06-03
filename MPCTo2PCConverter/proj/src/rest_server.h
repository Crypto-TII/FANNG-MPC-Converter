#ifndef __REST_SERVER_H__
#define __REST_SERVER_H__

#include <cpprest/http_listener.h>
#include <cpprest/json.h>

#include <functional>
#include <future>
#include <map>
#include <string>
#include <vector>

using web::http::http_request;
using web::http::http_response;
using web::http::methods;
using web::http::status_codes;
using web::http::experimental::listener::http_listener;
using web::http::experimental::listener::http_listener_config;
using web::uri_builder;

namespace json = web::json;

namespace net = boost::asio;
namespace ssl = net::ssl;

class RestServer {
protected:
  void display_json(json::value const &jvalue,
                    utility::string_t const &prefix) const;
  static std::string joinStrings(const std::vector<std::string> &arr,
                                 const std::string &sep = "");
  http_response handle_get(http_request request);

public:
  http_listener listener;
  std::map<std::vector<std::string>,
           std::function<http_response(http_request)>>
      get_routes;
  RestServer() = default;
  RestServer(const std::string &host, int port);

  ~RestServer() = default;
  void display_routes() const;

  static decltype(web::uri::split_query(utility::string_t("")))
  queryObjects(http_request request);
  [[noreturn]] void wait_for_request() const;
  void add_get_route(
      std::vector<std::string> const &path,
      std::function<http_response(http_request)> const
          &handler);
};

class RestHTTPSServer : public RestServer {
private:
  std::string cert;
  std::string privkey;

public:
  RestHTTPSServer(const std::string &host, int port, const std::string &cert,
                  const std::string &privkey);
};

#endif //__REST_SERVER_H__