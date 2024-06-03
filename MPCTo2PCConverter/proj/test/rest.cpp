#include <catch2/catch.hpp>

#include "../src/rest_client.h"
#include "../src/rest_server.h"

#include <linux/limits.h>
#include <string>
#include <unistd.h>

std::string do_readlink(std::string const &path) {
  char buff[PATH_MAX];
  if (ssize_t len = ::readlink(path.c_str(), buff, sizeof(buff) - 1);
      len != -1 && len < PATH_MAX) {
    buff[len] = '\0';
    return std::string(buff);
  }
  return "";
}

std::string get_selfpath() { return do_readlink("/proc/self/exe"); }

static const int _port = 43987; // should be available
static const std::string _host = "localhost";
static const std::string _cert = "test-certificate.pem";
static const std::string _privkey = "test-private-key.pem";

void init_server(RestServer &rest_server, const string &response) {
  std::atomic<bool> replied = false;
  rest_server.add_get_route(
      {"test"}, [&replied, &response](http_request request) {
        // create json response
        json::value response_json;
        response_json["msg"] = json::value::string(utility::string_t(response));
        request.reply(status_codes::OK, response_json);

        replied = true;
        return http_response(status_codes::OK);
      });
  int counter = 0;
  while (!replied && counter < 15) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
  }
}

TEST_CASE("REST http communication http_GET") {
  std::string host(_host);
  int port(_port);
  string expected_response("Hello, world!");

  std::thread server_thread([&host, port, &expected_response]() {
    RestServer rest_server(host, port);
    init_server(rest_server, expected_response);
  });

  // wait for server to start
  std::this_thread::sleep_for(std::chrono::seconds(2));

  RestClient rest_client(host, port);

  string response;
  rest_client.make_request(
      methods::GET, uri_builder().append_path(utility::string_t("/test")),
      [&response](json::value v) {
        response = v[utility::string_t("msg")].as_string();
      });

  server_thread.join();

  INFO("response = " << response)
  REQUIRE(response == expected_response);
}

TEST_CASE("REST https communication https_GET") {
  string dirname = get_selfpath();
  dirname = dirname.substr(0, dirname.find_last_of("/"));
  std::string host(_host);
  int port(_port);
  std::string cert(dirname + "/" + _cert);
  std::string privkey(dirname + "/" + _privkey);
  string expected_response("Hello, SECURE world!");

  std::thread server_thread(
      [&host, port, &cert, &privkey, &expected_response]() {
        RestHTTPSServer rest_server(host, port, cert, privkey);
        init_server(rest_server, expected_response);
      });

  // wait for server to start
  std::this_thread::sleep_for(std::chrono::seconds(2));

  RestHTTPSClient rest_client(host, port, dirname + "/" + _cert);

  string response;
  rest_client.make_request(
      methods::GET, uri_builder().append_path(utility::string_t("/test")),
      [&response](json::value v) {
        response = v[utility::string_t("msg")].as_string();
      });

  server_thread.join();

  INFO("response = " << response)
  REQUIRE(response == expected_response);
}
