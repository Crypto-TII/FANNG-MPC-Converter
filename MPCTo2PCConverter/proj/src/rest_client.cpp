/**
 * @file rest_client.cpp
 * @brief A simple REST client class using \ref
 * https://github.com/microsoft/cpprestsdk "Casablanca/cpprestsdk" library.
 * Usage:
 * @code
 #include "rest_client.h"
 int main(int argc, char *argv[]) {
  RestClient client("localhost", 9999);

  client.make_request(methods::GET,
                      uri_builder()
                          .append_path(U("/add"))
                          .append_query(U("arg1"), 3.1415)
                          .append_query(U("arg2"), 2.718),
                      [](json::value jvalue) {
                        cout << jvalue.serialize() << endl;
                        double result = jvalue.as_double();
                        cout << result << endl;
                      });

  return 0;
}

 * @endcode
 */
#include "rest_client.h"
#include <iostream>

using namespace web;
using namespace web::http;
using namespace web::http::client;

using std::clog;
using std::endl;
using std::to_string;

bool RestClient::error_status(const json::value &jvalue) {
  if (jvalue.has_field(utility::string_t("error_status"))) {
    return true;
  }
  return false;
}

string RestClient::error_message(json::value &jvalue) {
  if (jvalue.has_field(utility::string_t("error_message"))) {
    return jvalue[utility::string_t("error_message")].as_string();
  }
  return "";
}

http_client RestClient::get_client() {
  string url = "http://" + host + ":" + (to_string(port));
  auto uri = uri_builder(url).to_uri();
  http_client_config config;
  config.set_validate_certificates(false);
  http_client client(uri, config);
  return client;
}

RestClient::RestClient(const string &host, int port) : host(host), port(port) {}

void RestClient::make_request(const method &mtd, web::uri_builder const &uri,
                              const function<void(json::value)> &callback) {
  auto client = get_client();
  client.request(mtd, uri.to_string())
      .then([](http_response response) {
        if (response.status_code() == status_codes::OK) {
          return response.extract_json();
        }
        json::value jvalue;
        jvalue[utility::string_t("error_status")] =
            json::value::string(response.reason_phrase());
        return pplx::task_from_result(jvalue);
      })
      .then([&callback](pplx::task<json::value> previousTask) {
        try {
          auto const &jvalue = previousTask.get();
          callback(jvalue);
        } catch (http_exception const &e) {
          clog << e.what() << endl;
        }
      })
      .wait();
}

http_client RestHTTPSClient::get_client() {
  string url = "https://" + host + ":" + (to_string(port));
  http_client_config conf;
  try {
    conf.set_ssl_context_callback([this](boost::asio::ssl::context &ctx) {
      ctx.load_verify_file(this->cert);
    });
  } catch (std::exception const &e) {
    clog << "ERROR: " << e.what() << endl;
  }
  http_client client(uri_builder(url).to_uri(), conf);

  return client;
}
RestHTTPSClient::RestHTTPSClient(const string &host, int port,
                                 const string &cert)
    : RestClient(host, port), cert(cert) {}