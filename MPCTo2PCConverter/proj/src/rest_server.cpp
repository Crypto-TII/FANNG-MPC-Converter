#include "rest_server.h"

#include <chrono>
#include <iostream>
#include <thread>

using std::clog;
using std::cout;
using std::endl;
using std::exception;
using std::function;
using std::map;
using std::string;
using std::to_string;
using std::vector;

// NOTE: test REST server GET from the command line with curl:
// curl -G -v http://localhost:8080/echo -d "message=hello"
// for uri "http://localhost:8080/echo?message=hello"

void RestServer::display_json(json::value const &jvalue,
                              utility::string_t const &prefix) const {
  cout << prefix << jvalue.serialize() << endl;
}
string RestServer::joinStrings(const vector<string> &arr, const string &sep) {
  string out = arr[0];
  for (unsigned int i = 1; i < arr.size(); i++) {
    out += sep + arr[i];
  }
  return out;
}
http_response RestServer::handle_get(http_request request) {
  // parse query parameters
  const auto &uri = request.request_uri();
  auto requestPath = web::uri::split_path(uri.path());
  // dispatch based on the path
  auto pos = get_routes.find(requestPath);
  if (pos == get_routes.end()) {
    clog << "No route found for " << joinStrings(requestPath, "/") << endl;
    return http_response(status_codes::NotFound);
  } else {
    // call the handler
    return pos->second(request);
  }
}

// NOTE: easily add handlers for other HTTP methods
RestServer::RestServer(const string &host, int port) {
  listener = http_listener("http://" + host + ":" + to_string(port));
  listener.support(methods::GET, std::bind(&RestServer::handle_get, this,
                                           std::placeholders::_1));
  listener.open().wait();
  cout << "Listening for requests at: " << host << ":" << port << endl;
}
void RestServer::display_routes() const {
  cout << "contents of routes map: " << endl;
  for (const auto& [path, handler] : get_routes) {
    cout << joinStrings(path, "/") << " : <<handler>>" << endl;
  }
}

decltype(web::uri::split_query(utility::string_t("")))
RestServer::queryObjects(http_request request) {
  const auto &uri = request.request_uri();
  auto queryObjects = web::uri::split_query(uri.query());
  return queryObjects;
}
[[noreturn]] void RestServer::wait_for_request() const {
  while (true) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}
void RestServer::add_get_route(
    vector<string> const &path,
    function<http_response(http_request)> const &handler) {
  get_routes[path] = handler;
}

RestHTTPSServer::RestHTTPSServer(const string &host, int port,
                                 const string &cert, const string &privkey)
    : cert(cert), privkey(privkey) {
  string url = "https://" + host + ":" + to_string(port);
  http_listener_config conf;
  conf.set_ssl_context_callback([&cert, &privkey](ssl::context &ctx) {
    try {
      ctx.set_options(ssl::context::default_workarounds |
                      ssl::context::no_sslv2 | ssl::context::no_tlsv1 |
                      ssl::context::no_tlsv1_1 | ssl::context::single_dh_use);
      ctx.use_certificate_chain_file(cert);
      ctx.use_private_key_file(privkey, ssl::context::pem);
    } catch (exception const &e) {
      clog << "ERROR: " << e.what() << endl;
    }
  });
  listener = http_listener(utility::conversions::to_string_t(url), conf);
  listener.support(methods::GET, std::bind(&RestHTTPSServer::handle_get, this,
                                           std::placeholders::_1));
  listener.open().wait();
  cout << "Listening for requests at: " << host << ":" << port << endl;
}