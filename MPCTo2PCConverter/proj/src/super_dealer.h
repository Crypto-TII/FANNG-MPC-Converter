#ifndef SUPER_DEALER_H
#define SUPER_DEALER_H

#include <boost/program_options.hpp>
#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

#include <boost/program_options.hpp>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <tuple>
#include <string>
#include <vector>
#include <filesystem>
#include<yaml-cpp/yaml.h>


#include "enums.h"
#include "crypt_util.h"
#include "db_schemas.h"
#include "mysql.h"
#include <openssl/ssl.h>
#include <openssl/err.h>
#include "rest_server.h"
#include "tii_prime.h"
#include "postgresql.h"
#include "config_loader.h"


using std::make_shared;
using std::shared_ptr;
using std::string;
using std::stringstream;
using std::to_string;
using std::vector;

class Super_Dealer {
private:
    string sd_db_host;
    string sd_db_user;
    string sd_db_password;
    string sd_database;
    int sd_db_port;
    string sd_host;
    int sd_port;
    bool https;
    int db_system;
    string sd_cert;
    string sd_privkey;
    shared_ptr<RestServer> rest_server;

    http_response batch_id_get_handler(http_request request);

public:
    Super_Dealer(const string& sd_db_host, const string& sd_db_user,
                  const string& sd_db_password, const string& sd_database,
                  int sd_db_port, const string& sd_host,
                  int sd_port, bool https, const int& db_system,
                  const string& sd_cert, const string& sd_privkey);

    [[noreturn]] void start() const;
};

#endif // SUPER_DEALER_H
