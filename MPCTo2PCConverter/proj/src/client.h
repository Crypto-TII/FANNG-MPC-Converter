#ifndef CLIENT_H
#define CLIENT_H

#include "enums.h"
#include "crypt_util.h"
#include "db_schemas.h"
#include "mysql.h"
#include "postgresql.h"
#include "rest_client.h"
#include "tii_prime.h"
#include "config_loader.h"


#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <tuple>
#include <thread>
#include <vector>
#include <filesystem>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <cpprest/http_client.h>
#include<yaml-cpp/yaml.h>
#include <cryptopp/hex.h>
#include <boost/program_options.hpp>
#include <vector>
#include <string>
#include <tuple>
#include <filesystem>
#include <yaml-cpp/yaml.h>

using std::string;
using std::vector;
using std::tuple;

class Client {
private:
  // model owner params
  string client_db_host;
  string client_db_user;
  string client_db_password;
  string client_database;
  int client_db_port;
  bool https;
  int db_system;
  string mo_host;
  int mo_port;
  string mo_cert;
  vector<string> dealer_rest_hosts;
  vector<string> dealer_rest_certs;
  vector<int> dealer_rest_ports;

public:
  Client(const string &client_db_host, const string &client_db_user,
              const string &client_db_password, const string &client_database,
              const int client_db_port, bool https, const int &db_system,
              const string &mo_host, const int &mo_port, const string &mo_cert,
              const vector<string> &dealer_rest_hosts, const vector<string> &dealer_rest_certs,
              const vector<int> &dealer_rest_ports);

  tuple<string, int> get_batch_id(const int& model_id);
  tuple<vector<string>, vector<string>> get_shares_seed_and_mac_mask(const string& batch_id,const vector<int>& dealer_indices);
};

#endif // CLIENT_H