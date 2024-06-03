#ifndef __REST_CLIENT_H__
#define __REST_CLIENT_H__
/**
 * @file rest_client.h
 * @brief header for a simple REST client class.
 */
// following macro definition is to prevent
// compilation conflicts with boost
// (which defines the same "U" macro)
#define _TURN_OFF_PLATFORM_STRING
#include <cpprest/http_client.h>
#include <cpprest/json.h>

#include <functional>
#include <string>

namespace net = boost::asio;
namespace ssl = net::ssl;

using std::function;
using std::string;

class RestClient {
public:
  string host;
  int port;
  virtual web::http::client::http_client get_client();
  RestClient(const string &host, int port);
  virtual ~RestClient() = default;
  void make_request(const web::http::method &mtd, web::uri_builder const &uri,
                    const function<void(web::json::value)> &callback);
  static bool error_status(const web::json::value &jvalue);
  static std::string error_message(web::json::value &jvalue);
};

class RestHTTPSClient : public RestClient {
private:
  string cert;

public:
  web::http::client::http_client get_client() override;
  RestHTTPSClient(const string &host, int port, const string &cert);
  
};

#endif //__REST_CLIENT_H__
