#include "model_owner.h"

using namespace web;
using namespace web::http;
using namespace web::http::client;
using namespace std;

namespace opt = boost::program_options;

using std::condition_variable;
using std::make_shared;
using std::mutex;
using std::queue;
using std::scoped_lock;
using std::shared_ptr;
using std::string;
using std::stringstream;
using std::thread;
using std::to_string;
using std::unique_lock;
using std::vector;

using grpc::Channel;
using grpc::ClientContext;
using grpc::Status;
using shares::ShareRequest;
using shares::ShareResponse;
using shares::ShareDealer;

ShareDealerClient::ShareDealerClient(shared_ptr<Channel> channel)
    : stub(ShareDealer::NewStub(channel)) {}

vector<vector<string>> ShareDealerClient::GetShares(const string& batch_id) {
    ShareRequest request;
    request.set_batch_id(batch_id);
    ClientContext context;
    std::unique_ptr<grpc::ClientReader<ShareResponse>> reader(
        stub->GetShares(&context, request)
    );
    vector<vector<string>> shares;
    ShareResponse response;
    bool firstResponse = true; // To track the first response for mac_key_share
    
    while (reader->Read(&response)) {
        if (firstResponse) {
            mac_key_share = response.mac_key_share();
            firstResponse = false;
            continue;
        }
        std::vector<std::string> share_data;
        for (const auto& share_pair : response.share_pairs()) {
            share_data.push_back(share_pair.share());
            share_data.push_back(share_pair.mac_share());
        }
        shares.push_back(share_data);
    }

    if (auto status = reader->Finish(); !status.ok()) {
        std::cout << "Error: " << status.error_code() << ": " << status.error_message() << std::endl;
    }
    return shares;
}

string ShareDealerClient::GetMacKeyShare() const {
    return mac_key_share;
}


Model_Owner::Model_Owner(const string &mo_db_host, const string &mo_db_user,
                const string &mo_db_password, const string &mo_database,
                const int mo_db_port,
                const string &sd_host, const int &sd_port, const string &sd_cert,
                const string &mo_host, int &mo_port, bool https, const int &db_system, const string &mo_cert, 
                const string &mo_privkey, const int &max_share_size, const vector<int> &dealer_indices,
                const vector<shared_ptr<ShareDealerClient>> &grpc_dealer_clients)
    : mo_db_host(mo_db_host), mo_db_user(mo_db_user), mo_db_password(mo_db_password),
      mo_database(mo_database), mo_db_port(mo_db_port),
      sd_host(sd_host), sd_port(sd_port), sd_cert(sd_cert), mo_host(mo_host), mo_port(mo_port),
      https(https), db_system(db_system), mo_cert(mo_cert), mo_privkey(mo_privkey),
      max_share_size(max_share_size), dealer_indices(dealer_indices), grpc_dealer_clients(grpc_dealer_clients) {
    if (https) {
        rest_server = make_shared<RestHTTPSServer>(mo_host, mo_port, mo_cert, mo_privkey);
    } else {
        rest_server = make_shared<RestServer>(mo_host, mo_port);
    }

    // Add GET routes
    // /model_inference?model_id:str
    // `model_id` is the model id
    // returns a unique batch_id
    rest_server->add_get_route(
        {"model_inference"}, [this](http_request request) {
          return this->model_inference_get_handler(request);
        });
}

[[noreturn]] void Model_Owner::start() {
    thread t(&Model_Owner::query_dealers, this);
    // Start REST server
    rest_server->wait_for_request();
    t.join(); // Wait for query_dealers thread to finish
}

void Model_Owner::query_dealers() {
    size_t num_dealers = dealer_indices.size();
    while(true){
      unique_lock lk{query_dealers_mutex};
      // wait for notification
      // the predicate guarantees that batch_queue is not empty
      query_dealers_cv.wait(lk, [this] { return !batch_queue.empty(); });

      std::clog << "Querying dealers for batch_id = " << batch_queue.front()
                << " and model_id = " << model_queue.front() << " for " << total_shares_per_batch << " shares" << std::endl;
      string batch_id = batch_queue.front();
      string model_id = model_queue.front();
      
      vector<TiiPrime> shares(total_shares_per_batch);
      vector<TiiPrime> mac_shares(total_shares_per_batch);
      // logic to reconstruct shares from dealers
      TiiPrime mac_key_share_combined;
      for(int i = 0; i < num_dealers; i++){
        int total_shares_received = 0;
        int msg_start_idx = 0;
        int msg_end_idx = total_shares_per_batch;
        bool fragment = false;
        if (total_shares_per_batch > max_share_size) {
          fragment = true;
          msg_end_idx = max_share_size;
        }
        while(msg_end_idx <= total_shares_per_batch){
          bool success = false;
          while(!success){
            success = true;
            // shares reconstruction logic
            auto temp = grpc_dealer_clients[i]->GetShares(batch_id);
            //mac_key_share reconstruction logic
            auto mac_key_share = grpc_dealer_clients[i]->GetMacKeyShare();
            TiiPrime mks;
            stringstream(mac_key_share) >> mks;
            mac_key_share_combined += mks;
            if(temp.size() <= 0){
              std::cout << "No shares received from dealer " << dealer_indices[i] << std::endl;
              return;
            }
            total_shares_received += temp.size();
            std::cout << "Shares received from Dealer " << dealer_indices[i] << ": " << temp.size() << std::endl;
            TiiPrime x;
            for(int j = 0; j < total_shares_per_batch; j++){
              stringstream(temp[j][0]) >> x;
              shares[msg_start_idx+j] += x; 
              stringstream(temp[j][1]) >> x;
              mac_shares[msg_start_idx+j] += x;
            }
          }
          if (msg_end_idx == total_shares_per_batch) {
            break;
          }
          if (fragment) {
            msg_start_idx = msg_end_idx;
            msg_end_idx += total_shares_received;
            if (msg_end_idx > total_shares_per_batch) {
              msg_end_idx = total_shares_per_batch;
            }
          }
        }
    }
    // Insert the shares received from dealer to model owner's share table
    std::unique_ptr<PostgreSQL> postgresql;
    std::unique_ptr<MySQL> mysql;
    std::cout << "Updating batch table and types_per_batch table of MO with reconstructed shares and mac_key_share!" << std::endl;
    if (db_system == static_cast<int>(db_engine::mysql)){
      mysql = std::make_unique<MySQL>(mo_db_host, mo_db_user, mo_db_password, mo_database, mo_db_port);  
      vector<vector<string>> shares_and_mac_shares;
      for(int j = 0; j < total_shares_per_batch; j++){
        shares_and_mac_shares.push_back({shares[j].to_string(), mac_shares[j].to_string()});
      }
      mysql->insert_many("share", model_owner_shares_schema, shares_and_mac_shares);
      mysql->update("batch", "batch_id", batch_id, model_owner_batches_schema, 
                    {batch_id, model_id, mac_key_share_combined.to_string(), "1"});
    }else if (db_system == static_cast<int>(db_engine::postgres)){
      postgresql = std::make_unique<PostgreSQL>(mo_db_host, mo_db_user, mo_db_password, mo_database, mo_db_port);
      vector<vector<string>> shares_and_mac_shares;
      for(int j = 0; j < total_shares_per_batch; j++){
        shares_and_mac_shares.push_back({shares[j].to_string(), mac_shares[j].to_string()});
      }
      postgresql->insert_many(mo_database, "share", model_owner_shares_schema, shares_and_mac_shares);
      postgresql->update(mo_database, "batch", "batch_id", batch_id, model_owner_batches_schema, 
                    {batch_id, model_id, mac_key_share_combined.to_string(), "1"});\
    }
      batch_queue.pop();
      model_queue.pop();
      std::cout << "Done Filling MO's tables!" << std::endl;
    }
}

void Model_Owner::signal_query_thread(const std::string &model_id, const std::string &batch_id) {
    // Signal thread to query dealers for shares
    // Client will query dealers for seeds,
    // which will be available after dealers have generated shares
    unique_lock lock(query_dealers_mutex);
    batch_queue.push(batch_id);
    model_queue.push(model_id);
    query_dealers_cv.notify_one();
}

http_response Model_Owner::model_inference_get_handler(http_request request) {
    // Model inference GET request handler
    auto queryObjects = RestServer::queryObjects(request);
    // check if all required parameters are present
    if (queryObjects.find("model_id") == queryObjects.end()) {
        std::clog << "missing required model_id parameter\n";
        http_response response(status_codes::BadRequest);
        stringstream reason;
        const auto &uri = request.request_uri();
        reason << "REST uri " << uri.path() << "/" << uri.query()
                << " missing required parameter: model_id";
        response.set_reason_phrase(reason.str());
        request.reply(status_codes::BadRequest);
        return response;
    }
    string model_id = queryObjects["model_id"];
    // get model from database
    vector<vector<string>> model_rows;
    std::unique_ptr<PostgreSQL> postgresql;
    std::unique_ptr<MySQL> mysql;
    if (db_system == static_cast<int>(db_engine::mysql)){
      mysql = std::make_unique<MySQL>(mo_db_host, mo_db_user, mo_db_password, mo_database, mo_db_port);  
      model_rows = mysql->parse_result(mysql->find_rows("model", "model_id", model_id),
                                        model_owner_models_schema);  
    }else if (db_system == static_cast<int>(db_engine::postgres)){
      postgresql = std::make_unique<PostgreSQL>(mo_db_host, mo_db_user, mo_db_password, mo_database, mo_db_port);
      model_rows = postgresql->parse_result(postgresql->find_rows(mo_database, "model", "model_id", model_id),
                                        model_owner_models_schema);      
    }else{
      http_response response(status_codes::BadRequest);
      stringstream reason;
      std::clog << "Value for parameter db can only be '0' or '1'\n";
      reason << "Value for parameter db can only be '0' or '1'";
      response.set_reason_phrase(reason.str());
      request.reply(status_codes::NotFound);
      return response;
    }
    if (model_rows.empty()) {
      http_response response(status_codes::BadRequest);
      stringstream reason;
      const auto &uri = request.request_uri();
      reason << "REST uri " << uri.path() << "/" << uri.query();
      std::clog << "model not found\n";
      reason << " model not found: " << model_id;
      response.set_reason_phrase(reason.str());
      request.reply(status_codes::NotFound);
      return response;
    }
    // Make request to Super Dealer for an unused batch
    string batch_id;
    shared_ptr<RestClient> clientOfSuperDealer;
    if (https) {
      clientOfSuperDealer = make_shared<RestHTTPSClient>(sd_host, sd_port, sd_cert);

    } else {
      clientOfSuperDealer = make_shared<RestClient>(sd_host, sd_port);
    }
    clientOfSuperDealer->make_request(web::http::methods::GET,
                        web::http::uri_builder()
                            .append_path(utility::string_t("/batch_id"))
                            .append_query(utility::string_t("model_id"), model_id),
                        [&batch_id](web::json::value v) {
                            if (v.has_field("batch_id")) {
                              batch_id = v[utility::string_t("batch_id")].as_string();
                            }
                        });    
    if(batch_id.empty()){
      http_response response(status_codes::BadRequest);
      stringstream reason;
      const auto &uri = request.request_uri();
      reason << "REST uri " << uri.path() << "/" << uri.query();
      std::clog << "no batches for requested model\n";
      reason << " no batches for requested model: " << model_id;
      response.set_reason_phrase(reason.str());
      request.reply(status_codes::NotFound);
      return response;
    }
    // insert the new batch into database (batches table) and fill types_per_batch table for MO
    total_shares_per_batch = 0;
    int row = 1;
    int order_in_batch = 1;
    vector<vector<string>> types_per_model_rows;
    vector<vector<string>> material_type_rows;
    if (db_system == static_cast<int>(db_engine::mysql)){
      mysql = std::make_unique<MySQL>(mo_db_host, mo_db_user, mo_db_password, mo_database, mo_db_port);  
      mysql->insert_one("batch", model_owner_batches_schema,
                {batch_id,  model_id, "-1", "0"});
      types_per_model_rows = mysql->parse_result(mysql->find_rows("types_per_model", "model_id", model_id),
                                        model_owner_types_per_model_schema); 
      for(auto& rows: types_per_model_rows){
        string material_type_id = rows[2];
        material_type_rows = mysql->parse_result(mysql->find_rows("material_type", "material_type_id", 
                              material_type_id), model_owner_material_type_schema); 
        string material_name = material_type_rows[0][1];
        int num_materials = get_material_size(material_name, stoi(rows[1]));
        total_shares_per_batch += num_materials; 
        mysql->insert_one("types_per_batch", model_owner_types_per_batch_schema, 
                  {batch_id, material_type_id,
                  to_string(row), to_string(row+num_materials), to_string(order_in_batch)});
        row += num_materials;
        order_in_batch++;
      }
    }else if (db_system == static_cast<int>(db_engine::postgres)){
      postgresql = std::make_unique<PostgreSQL>(mo_db_host, mo_db_user, mo_db_password, mo_database, mo_db_port);  
      postgresql->insert_one(mo_database, "batch", model_owner_batches_schema,
                {batch_id,  model_id, "-1", "0"});
      types_per_model_rows = postgresql->parse_result(postgresql->find_rows(mo_database, "types_per_model", "model_id", model_id),
                                        model_owner_types_per_model_schema);   
      for(auto& rows: types_per_model_rows){
        string material_type_id = rows[2];
        material_type_rows = postgresql->parse_result(postgresql->find_rows(mo_database, "material_type", "material_type_id", 
                              material_type_id), model_owner_material_type_schema); 
        string material_name = material_type_rows[0][1];
        int num_materials = get_material_size(material_name, stoi(rows[1]));
        total_shares_per_batch += num_materials; 
        postgresql->insert_one(mo_database, "types_per_batch", model_owner_types_per_batch_schema, 
                  {batch_id, material_type_id,
                  to_string(row), to_string(row+num_materials), to_string(order_in_batch)});
        row += num_materials;
        order_in_batch++;
      } 
    }
    signal_query_thread(model_id, batch_id);
    // create json response
    json::value response_json;
    response_json["batch_id"] = json::value::string(batch_id);

    request.reply(status_codes::OK, response_json);
    return http_response(status_codes::OK);
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
  string currentDir = std::filesystem::current_path().parent_path().string();
  YAML::Node config = ConfigLoader::loadConfig(currentDir+"/config/config.yaml");
  // clang-format off
  int mo_id;
  opt::options_description desc("dealer converter command line options");
  desc.add_options()
    ("id", opt::value<int>(&mo_id), "ID of client")
    ("help", "display help message");
    
  // parse command line arguments
  opt::variables_map vm;
  opt::store(opt::parse_command_line(argc, argv, desc), vm);
  opt::notify(vm);

  if (!vm.count("id") || !check_valid_id(config, mo_id, "model_owners")) {
    std::cout << "Invalid entry for ID! Usage Options: --id <id>";
    return 1;
  }
  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }
  YAML::Node mo_config = ConfigLoader::getModelOwnerConfig(config, mo_id);
  bool https = mo_config["https"].as<bool>();
  int db_system = mo_config["db_engine"].as<int>();
  string dealer_ids_str = mo_config["dealer_ids_str"].as<string>();

  int max_share_size = mo_config["max_share_size"].as<int>();
  auto mo_cert = mo_config["mo_cert"].as<string>();
  auto mo_privkey = mo_config["mo_privkey"].as<string>();
  auto mo_host = mo_config["mo_host"].as<string>();
  int mo_port = mo_config["mo_port"].as<int>();
  string db_str = "mysql";
  if (db_system == static_cast<int>(db_engine::postgres)){
    db_str = "postgres";
  }
  auto mo_db_host = mo_config["mo_"+db_str+"_host"].as<string>();
  int mo_db_port = mo_config["mo_"+db_str+"_port"].as<int>();
  auto mo_db_user = mo_config["mo_"+db_str+"_user"].as<string>();
  auto mo_db_password = mo_config["mo_"+db_str+"_password"].as<string>();
  auto mo_database = mo_config["mo_"+db_str+"_database"].as<string>();

  vector<string> grpc_hosts;
  vector<int> grpc_ports;
  vector<string> grpc_certs;
  vector<string> grpc_privkey;
  int num_dealers = mo_config["num_dealers"].as<int>();
  for(int i = 0; i < num_dealers; i++){
    YAML::Node dealer_config = ConfigLoader::getDealerConfig(config, i);
    grpc_hosts.emplace_back(dealer_config["host"].as<string>());
    grpc_ports.push_back(dealer_config["port"].as<int>());
    grpc_certs.emplace_back(dealer_config["grpc_cert"].as<string>());
  }
  dealer_ids_str = mo_config["dealer_ids_str"].as<string>();
  auto sd_host = mo_config["sd_host"].as<string>();
  int sd_port = mo_config["sd_port"].as<int>();
  auto sd_cert = mo_config["sd_cert"].as<string>();

  vector<int> dealer_indices;
  for(char c : dealer_ids_str){
    if(!std::isdigit(c))
      continue;
    int dealer_id = c - '0';
    if(dealer_id < 0 || dealer_id >= num_dealers){
      std::cout << "Invalid Dealer Index!";
      return 1;
    }
    dealer_indices.push_back(dealer_id);
  }
  // Initialize our grpc dealers!
  vector<shared_ptr<ShareDealerClient>> grpc_dealer_clients(dealer_indices.size());
  for (int i = 0; i < dealer_indices.size(); i++) {
    auto creds = grpc::InsecureChannelCredentials();
    if (https){
      grpc::SslCredentialsOptions ssl_opts;
      ssl_opts.pem_root_certs = read_keycert(grpc_certs[dealer_indices[i]]);
      creds = grpc::SslCredentials(ssl_opts);
    }
    std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel((grpc_hosts[dealer_indices[i]]+":"+
                                                                  std::to_string(grpc_ports[dealer_indices[i]])), creds);
    grpc_dealer_clients[i] = make_shared<ShareDealerClient>(channel);
  }
  std::cout << "Starting servers!" << std::endl;
  Model_Owner mo(mo_db_host, mo_db_user, mo_db_password, mo_database,
              mo_db_port, sd_host, sd_port, sd_cert,
              mo_host, mo_port, https, db_system, mo_cert, mo_privkey, max_share_size, dealer_indices, grpc_dealer_clients);
  mo.start();
  return 0;
}