#include "dealer.h"

namespace opt = boost::program_options;

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

vector<string> mask_shares(const vector<string> &shares,
                           shared_ptr<AES_RNG> prg) {
  vector<string> masked_shares(shares.size());
  CryptoPP::SecByteBlock mask_bytes(field_size * shares.size());
  prg->GenerateBlock(mask_bytes, mask_bytes.size());
  auto mask_128 = reinterpret_cast<__int128 *>(mask_bytes.data());
  TiiPrime x;
  for (int i = 0; i < shares.size(); i++) {
    TiiPrime mask(ntoh128(mask_128[i]), true);
    stringstream ss(shares[i]);
    ss >> x;
    x -= mask;
    masked_shares[i] = x.to_string();
  }
  return masked_shares;
}
tuple<string, string> mask_mackey(const string& mackey, shared_ptr<AES_RNG> prg){
   //mask mac_key_share
    CryptoPP::SecByteBlock mask_bytes(field_size);
    prg->GenerateBlock(mask_bytes, mask_bytes.size());
    auto mask_128 = reinterpret_cast<__int128 *>(mask_bytes.data());
    TiiPrime x;
    TiiPrime mask(ntoh128(mask_128[0]), true);
    stringstream ss(mackey);
    ss >> x;
    x -= mask;
    return {x.to_string(), mask.to_string()};
}

Status Dealer::GetShares(ServerContext* context, const ShareRequest* request, grpc::ServerWriter<ShareResponse>* writer) {
    string batch_id = request->batch_id();
    if (batch_id.empty()) {
      return Status(grpc::StatusCode::INVALID_ARGUMENT, "Invalid batch_id");
    }
    // logic to get and store shares 
    vector<vector<string>> types_per_batch_rows;
    string mac_key_share;
    string shares_seed_temp;
    string mac_key_mask_temp;
    std::unique_ptr<PostgreSQL> postgresql;
    std::unique_ptr<MySQL> mysql;
    string model_id;
    if (db_system == static_cast<int>(db_engine::mysql)){
      mysql = std::make_unique<MySQL>(dealer_db_host, dealer_db_user, dealer_db_password, dealer_database, dealer_db_port);  
      types_per_batch_rows = mysql->parse_result(mysql->find_rows("types_per_batch", "batch_id", batch_id), dealer_types_per_batch_schema);  
      auto result = mysql->parse_result(mysql->find_rows("batch", "batch_id", batch_id), dealer_batches_schema);  
      model_id = result[0][1];
      mac_key_share = result[0][2];
      shares_seed_temp = result[0][3];
      mac_key_mask_temp = result[0][4];
    }else if (db_system == static_cast<int>(db_engine::postgres)){
      postgresql = std::make_unique<PostgreSQL>(dealer_db_host, dealer_db_user, dealer_db_password, dealer_database, dealer_db_port);
      types_per_batch_rows = postgresql->parse_result(postgresql->find_rows(dealer_database, "types_per_batch", "batch_id", batch_id), dealer_types_per_batch_schema);   
      auto result = postgresql->parse_result(postgresql->find_rows(dealer_database, "batch", "batch_id", batch_id), dealer_batches_schema);    
      model_id = result[0][1];
      mac_key_share = result[0][2];
      shares_seed_temp = result[0][3];
      mac_key_mask_temp = result[0][4];
    }else{
      return Status(grpc::StatusCode::INVALID_ARGUMENT, "Invalid db option selected, Available: 0: MySQL, 1: PostgreSQL");
    }
    CryptoPP::SecByteBlock shares_seed;
    // mask mac_key_share
    shared_ptr<AES_RNG>  mackey_prg;
    string masked_mac_key_share;
    string mac_key_mask;
    if(types_per_batch_rows.empty()){
      return Status(grpc::StatusCode::NOT_FOUND, "No results obtained for given batch_id");
    }else{
      if(shares_seed_temp == "null"){    
        std::cout << "No seed for batch_id " << batch_id << " available at dealer! GENERATING SEED!"<<std::endl;
        shares_seed = generate_seed();
        mackey_prg = generate_prg(generate_seed());
        std::tie(masked_mac_key_share, mac_key_mask) = mask_mackey(mac_key_share, mackey_prg);
        // Update seed in table for dealers!
        if (db_system == static_cast<int>(db_engine::mysql)){
          mysql = std::make_unique<MySQL>(dealer_db_host, dealer_db_user, dealer_db_password, dealer_database, dealer_db_port);  
          mysql->update("batch", "batch_id", batch_id, dealer_batches_schema, 
                        {batch_id, model_id, mac_key_share, seed2string(shares_seed), mac_key_mask});
        }else if (db_system == static_cast<int>(db_engine::postgres)){
          postgresql = std::make_unique<PostgreSQL>(dealer_db_host, dealer_db_user, dealer_db_password, dealer_database, dealer_db_port);
          postgresql->update(dealer_database, "batch", "batch_id", batch_id, dealer_batches_schema, 
                        {batch_id, model_id, mac_key_share, seed2string(shares_seed), mac_key_mask});
        }
      }else{ // This is a case where seed was filled in the /seed request made by the clients
        shares_seed = string2seed(shares_seed_temp);
        mac_key_mask = mac_key_mask_temp;
        TiiPrime x1;
        stringstream ss1(mac_key_mask);
        ss1 >> x1;
        TiiPrime x2;
        stringstream ss2(mac_key_share);
        ss2 >> x2;
        x2 -= x1;
        masked_mac_key_share = x2.to_string();
      }
    }
    sortByColumnIndex(types_per_batch_rows, 4); // 4 is the index of order_in_batch column
    vector<string> shares_vec;
    vector<string> mac_shares_vec;
    for(auto& it: types_per_batch_rows){
      int start_row = stoi(it[2]);
      int end_row = stoi(it[3]);
      vector<vector<string>> share_rows;
      if (db_system == static_cast<int>(db_engine::mysql)){
        mysql = std::make_unique<MySQL>(dealer_db_host, dealer_db_user, dealer_db_password, dealer_database, dealer_db_port);  
        auto result = mysql->get_rows("share", start_row, end_row, dealer_shares_schema);
        share_rows = mysql->parse_result(result, dealer_shares_schema); 
      }else if (db_system == static_cast<int>(db_engine::postgres)){
        postgresql = std::make_unique<PostgreSQL>(dealer_db_host, dealer_db_user, dealer_db_password, dealer_database, dealer_db_port);
        auto result = postgresql->get_rows(dealer_database, "share", start_row, end_row, dealer_shares_schema);
        share_rows = postgresql->parse_result(result, dealer_shares_schema);      
      }else{
        return Status(grpc::StatusCode::INVALID_ARGUMENT, "Invalid db option selected, Available: 0: MySQL, 1: PostgreSQL");
      }
      for (auto &row : share_rows) {
        shares_vec.push_back(row[0]);
        mac_shares_vec.push_back(row[1]);
      }
    }
    // mask shares
    auto shares_prg = generate_prg(shares_seed);
    auto masked_shares = mask_shares(shares_vec, shares_prg);
    auto masked_mac_shares = mask_shares(mac_shares_vec, shares_prg);
    // Split shares into batches based on the size limit
    for (size_t i = 0; i < masked_shares.size(); i += max_share_size) {
      size_t end = std::min(i + max_share_size, masked_shares.size());
      vector<string> batch_shares(masked_shares.begin() + i, masked_shares.begin() + end);
      vector<string> batch_mac_shares(masked_mac_shares.begin() + i, masked_mac_shares.begin() + end);
      ShareResponse response;
      if (i == 0) {
          // Assuming the masked_mac_key_share needs to be sent once at the start of the stream
          response.set_mac_key_share(masked_mac_key_share); 
          if (!writer->Write(response)) {
              return Status(grpc::StatusCode::CANCELLED, "Error while sending mac_key_share");
          }
      }
      for (int j = 0; j < batch_shares.size(); j++) {
        ShareResponse::SharePair* pair = response.add_share_pairs();
        pair->set_share(batch_shares[j]);
        pair->set_mac_share(batch_mac_shares[j]);

        if (!writer->Write(response)) {
            return Status(grpc::StatusCode::CANCELLED, "Error while sending shares");
        }
        response.clear_share_pairs(); // Clear the repeated field for the next element
      }
    }
    return Status::OK;
}

bool Dealer::sortByColumn(const vector<string>& row1, const vector<string>& row2, int columnIdx) const {
    return std::stoi(row1[columnIdx]) < std::stoi(row2[columnIdx]);
}

void Dealer::sortByColumnIndex(vector<vector<string>>& data, int columnIdx) const {
    std::sort(data.begin(), data.end(), [this, columnIdx](const vector<string> &row1, const vector<string> &row2) {
          return sortByColumn(row1, row2, columnIdx);
    });
}

web::http::http_response Dealer::seed_get_handler(web::http::http_request request) const {
    std::clog << "Dealer - send seed of batch to client\n";
    auto queryObjects = RestServer::queryObjects(request);
    // check if all required parameters are present
    if (queryObjects.find("batch_id") == queryObjects.end()) {
      std::clog << "missing required batch_id parameter\n";
      http_response response(status_codes::BadRequest);
      stringstream reason;
      const auto &uri = request.request_uri();
      reason << "REST uri " << uri.path() << "/" << uri.query()
             << " missing required parameter: batch_id" << std::endl;
      response.set_reason_phrase(reason.str());
      request.reply(status_codes::BadRequest);
      return response;
    }
    string batch_id = queryObjects["batch_id"];
    std::unique_ptr<PostgreSQL> postgresql;
    std::unique_ptr<MySQL> mysql;
    string shares_seed;
    string mac_key_mask;
    string mac_key_share;
    string model_id;
    if (db_system == static_cast<int>(db_engine::mysql)){
      mysql = std::make_unique<MySQL>(dealer_db_host, dealer_db_user, dealer_db_password, dealer_database, dealer_db_port);  
      auto result = mysql->parse_result(mysql->find_rows("batch", "batch_id", batch_id), dealer_batches_schema);
      model_id = result[0][1];
      mac_key_share = result[0][2];
      shares_seed = result[0][3];
      mac_key_mask = result[0][4];
    }else if (db_system == static_cast<int>(db_engine::postgres)){
      postgresql = std::make_unique<PostgreSQL>(dealer_db_host, dealer_db_user, dealer_db_password, dealer_database, dealer_db_port);
      auto result = postgresql->parse_result(postgresql->find_rows(dealer_database, "batch", "batch_id", batch_id), dealer_batches_schema); 
      model_id = result[0][1];
      mac_key_share = result[0][2];
      shares_seed = result[0][3];
      mac_key_mask = result[0][4];
    }
    string masked_mac_key_share;
    if(shares_seed == "null"){
      std::clog << "seed preparation not done" << std::endl;
      http_response response(status_codes::BadRequest);
      response.set_reason_phrase("seed not found in database");
      request.reply(status_codes::NotFound);
      return response;
    }
    json::value response_json;
    response_json["shares_seed"] = json::value::string(utility::string_t(shares_seed));
    response_json["mac_key_mask"] = json::value::string(utility::string_t(mac_key_mask));
    request.reply(status_codes::OK, response_json);
    return http_response(status_codes::OK);
  }

Dealer::Dealer(bool https, const std::string& dealer_db_host, const std::string& dealer_db_user,
               const std::string& dealer_db_password, const int& dealer_db_port,
               const std::string& dealer_database, const std::string& dealer_grpc_host,
               const int& dealer_grpc_port, const int& db_system,
               const int& max_share_size, const std::string& dealer_rest_host,
               const int& dealer_rest_port, const std::string& dealer_rest_cert,
               const std::string& dealer_rest_privkey, const std::string& dealer_grpc_cert,
               const std::string& dealer_grpc_privkey) :
    https(https), dealer_db_host(dealer_db_host), dealer_db_user(dealer_db_user),
    dealer_db_password(dealer_db_password), dealer_db_port(dealer_db_port),
    dealer_database(dealer_database), dealer_grpc_host(dealer_grpc_host),
    dealer_grpc_port(dealer_grpc_port), db_system(db_system),
    max_share_size(max_share_size), dealer_rest_host(dealer_rest_host),
    dealer_rest_port(dealer_rest_port), dealer_rest_cert(dealer_rest_cert),
    dealer_rest_privkey(dealer_rest_privkey), dealer_grpc_cert(dealer_grpc_cert),
    dealer_grpc_privkey(dealer_grpc_privkey) {

    if (https) {
        rest_server = std::make_shared<RestHTTPSServer>(dealer_rest_host, dealer_rest_port, dealer_rest_cert, dealer_rest_privkey);
    } else {
        rest_server = std::make_shared<RestServer>(dealer_rest_host, dealer_rest_port);
    }
    rest_server->add_get_route({"seed"}, [this](web::http::http_request request) {
        return this->seed_get_handler(request);
    });
}

void Dealer::start_grpc() {
    string server_address(dealer_grpc_host + ":" + std::to_string(dealer_grpc_port));
    ServerBuilder builder;

    auto creds = grpc::InsecureServerCredentials();
    if (https) {
        grpc::SslServerCredentialsOptions::PemKeyCertPair pkcp;
        pkcp.private_key = read_keycert(dealer_grpc_privkey);
        pkcp.cert_chain = read_keycert(dealer_grpc_cert);
        grpc::SslServerCredentialsOptions ssl_opts;
        ssl_opts.pem_root_certs = "";
        ssl_opts.pem_key_cert_pairs.push_back(pkcp);
        creds = grpc::SslServerCredentials(ssl_opts);
    }
    builder.AddListeningPort(server_address, creds);
    builder.RegisterService(this);
    std::unique_ptr<Server> server(builder.BuildAndStart());
    std::cout << "Server listening on " << server_address << std::endl;
    server->Wait();
}

[[noreturn]] void Dealer::start_servers() {
    std::thread grpc_thread([this]() {
        start_grpc();
    });
    grpc_thread.join();
    rest_server->wait_for_request();
}

bool check_valid_id(const YAML::Node& config, const int& id, const string& actor){
  vector<int> available_ids;
  const YAML::Node& actor_config = config[actor];
  for (const auto& ids : actor_config) {
      available_ids.push_back(ids["id"].as<int>());
  }
  auto it = std::find(available_ids.begin(), available_ids.end(), id);
  return (it != available_ids.end());
}

int main(int argc, char *argv[]) {
  int dealer_id;
  // Load the config of the dealer whose id has been provided
  string currentDir = std::filesystem::current_path().parent_path().string();
  YAML::Node config_all = ConfigLoader::loadConfig(currentDir+"/config/config.yaml");
  opt::options_description desc("dealer converter command line options");
  // clang-format off
  desc.add_options()
    ("id", opt::value<int>(&dealer_id), "dealer id: 0 to 9")
    ("help", "display help message");

  // parse command line arguments
  opt::variables_map vm;
  opt::store(opt::parse_command_line(argc, argv, desc), vm);
  opt::notify(vm);
 
  if (!vm.count("id") || !check_valid_id(config_all, dealer_id, "dealers")) {
    std::cout << "Invalid entry for ID! Usage Options: --id <id>";
    return 1;
  }
  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }
  YAML::Node config = ConfigLoader::getDealerConfig(config_all, dealer_id);

  bool https = config["https"].as<bool>();
  int db_system = config["db_engine"].as<int>();
  int max_share_size = config["max_share_size"].as<int>();
  auto dealer_grpc_host = config["host"].as<string>();
  int dealer_grpc_port = config["port"].as<int>();
  string db_str = "mysql";
  if (db_system == static_cast<int>(db_engine::postgres)){
    db_str = "postgres";
  }
  auto dealer_db_host = config[db_str+"_host"].as<string>();
  int dealer_db_port = config[db_str+"_port"].as<int>();
  auto dealer_db_user = config[db_str+"_user"].as<string>();
  auto dealer_db_password = config[db_str+"_pwd"].as<string>();
  auto dealer_database = config[db_str+"_db"].as<string>();

  string dealer_rest_host = config["rest_host"].as<string>();
  int dealer_rest_port = config["rest_port"].as<int>();
  string dealer_rest_cert = config["rest_cert"].as<string>();
  string dealer_rest_privkey = config["rest_privkey"].as<string>();

  string dealer_grpc_cert = config["grpc_cert"].as<string>();
  string dealer_grpc_privkey = config["grpc_privkey"].as<string>();
  
  Dealer dealer(https, dealer_db_host, dealer_db_user, dealer_db_password, dealer_db_port,
                dealer_database, dealer_grpc_host, dealer_grpc_port, db_system, max_share_size, 
                dealer_rest_host, dealer_rest_port, dealer_rest_cert, dealer_rest_privkey, 
                dealer_grpc_cert, dealer_grpc_privkey);
  dealer.start_servers();
  return 0;
}