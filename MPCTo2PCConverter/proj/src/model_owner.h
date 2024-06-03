#ifndef MODEL_OWNER_H
#define MODEL_OWNER_H

#include <boost/program_options.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <condition_variable>
#include <functional>
#include <future>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <string>
#include <thread>
#include <vector>
#include <filesystem>
#include <grpcpp/grpcpp.h>
#include <yaml-cpp/yaml.h>
#include <iostream>
#include <sstream>

#include "enums.h"
#include "crypt_util.h"
#include "db_schemas.h"
#include "mysql.h"
#include "postgresql.h"
#include "rest_client.h"
#include "rest_server.h"
#include "tii_prime.h"
#include "shares.grpc.pb.h"
#include "shares.pb.h"
#include "config_loader.h"

using std::string;
using std::vector;
using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using shares::ShareRequest;
using shares::ShareResponse;
using shares::ShareDealer;

class ShareDealerClient {
private:
    std::unique_ptr<ShareDealer::Stub> stub;
    std::string mac_key_share;

public:
    explicit ShareDealerClient(std::shared_ptr<Channel> channel);

    vector<vector<string>> GetShares(const string& batch_id);

    string GetMacKeyShare() const;
};

class Model_Owner {
private:
    // model owner params
    std::string mo_db_host;
    std::string mo_db_user;
    std::string mo_db_password;
    std::string mo_database;
    int mo_db_port;

    std::string sd_host;
    int sd_port;
    std::string sd_cert;

    std::string mo_host;
    int mo_port;
    bool https;
    int db_system;
    std::string mo_cert;
    std::string mo_privkey;

    int max_share_size;
    int total_shares_per_batch;
    std::vector<int> dealer_indices;
    const std::vector<std::shared_ptr<ShareDealerClient>> &grpc_dealer_clients;

    std::shared_ptr<RestServer> rest_server;

    std::queue<std::string> batch_queue;
    std::queue<std::string> model_queue;
    std::atomic<bool> running = true;
    std::mutex query_dealers_mutex;
    std::condition_variable query_dealers_cv;

    void signal_query_thread(const std::string &model_id, const std::string &batch_id);

    web::http::http_response model_inference_get_handler(web::http::http_request request);

public:
    Model_Owner(const std::string &mo_db_host, const std::string &mo_db_user,
                const std::string &mo_db_password, const std::string &mo_database,
                const int mo_db_port,
                const std::string &sd_host, const int &sd_port, const std::string &sd_cert,
                const std::string &mo_host, int &mo_port, bool https, const int &db_system, const std::string &mo_cert, 
                const std::string &mo_privkey, const int &max_share_size, const std::vector<int> &dealer_indices,
                const std::vector<std::shared_ptr<ShareDealerClient>> &grpc_dealer_clients);

    [[noreturn]] void start();

    void query_dealers();
};

#endif // MODEL_OWNER_H
