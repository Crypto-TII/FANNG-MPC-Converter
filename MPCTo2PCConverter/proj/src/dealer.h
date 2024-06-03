#ifndef DEALER_H
#define DEALER_H

#include <boost/program_options.hpp>
#include <grpcpp/grpcpp.h>
#include <iostream>
#include <memory>
#include <string>
#include <thread>
#include <vector>
#include <cpprest/http_listener.h>
#include <cpprest/uri.h>
#include <filesystem>

#include "enums.h"
#include "crypt_util.h"
#include "db_schemas.h"
#include "mysql.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "rest_server.h"
#include "tii_prime.h"
#include "postgresql.h"
#include "shares.grpc.pb.h"
#include "shares.pb.h"
#include "config_loader.h"

using std::make_shared;
using std::shared_ptr;
using std::string;
using std::stringstream;
using std::to_string;
using std::vector;
using std::tuple;

using grpc::Server;
using grpc::ServerBuilder;
using grpc::ServerContext;
using grpc::Status;
using shares::ShareRequest;
using shares::ShareResponse;
using shares::ShareDealer;

class Dealer final : public ShareDealer::Service {
private:
    bool https;
    string dealer_db_host;
    string dealer_db_user;
    string dealer_db_password;
    int dealer_db_port;
    string dealer_database;
    string dealer_grpc_host;
    int dealer_grpc_port;
    int db_system;
    int max_share_size;
    string dealer_rest_host;
    int dealer_rest_port;
    string dealer_rest_cert;
    string dealer_rest_privkey;
    string dealer_grpc_cert;
    string dealer_grpc_privkey;
    shared_ptr<RestServer> rest_server;

    Status GetShares(ServerContext* context, const ShareRequest* request, grpc::ServerWriter<ShareResponse>* writer) override;
    bool sortByColumn(const vector<string>& row1, const vector<string>& row2, int columnIdx) const;
    void sortByColumnIndex(vector<vector<string>>& data, int columnIdx) const;
    web::http::http_response seed_get_handler(web::http::http_request request) const;

public:
    Dealer(bool https, const std::string& dealer_db_host, const std::string& dealer_db_user,
           const std::string& dealer_db_password, const int& dealer_db_port,
           const std::string& dealer_database, const std::string& dealer_grpc_host,
           const int& dealer_grpc_port, const int& db_system,
           const int& max_share_size, const std::string& dealer_rest_host,
           const int& dealer_rest_port, const std::string& dealer_rest_cert,
           const std::string& dealer_rest_privkey, const std::string& dealer_grpc_cert,
           const std::string& dealer_grpc_privkey);

    void start_grpc();
    [[noreturn]] void start_servers();
};

#endif // DEALER_H
