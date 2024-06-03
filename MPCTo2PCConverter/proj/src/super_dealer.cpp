#include "super_dealer.h"

namespace opt = boost::program_options;

using std::make_shared;
using std::shared_ptr;
using std::string;
using std::stringstream;
using std::to_string;
using std::vector;

Super_Dealer::Super_Dealer(const string& sd_db_host, const string& sd_db_user,
                           const string& sd_db_password, const string& sd_database,
                           int sd_db_port, const string& sd_host,
                           int sd_port, bool https, const int& db_system,
                           const string& sd_cert, const string& sd_privkey)
    : sd_db_host(sd_db_host), sd_db_user(sd_db_user), sd_db_password(sd_db_password),
      sd_database(sd_database), sd_db_port(sd_db_port), sd_host(sd_host), sd_port(sd_port),
      https(https), db_system(db_system), sd_cert(sd_cert), sd_privkey(sd_privkey) {
    if (https) {
        rest_server = make_shared<RestHTTPSServer>(sd_host, sd_port, sd_cert, sd_privkey);
    } else {
        rest_server = make_shared<RestServer>(sd_host, sd_port);
    }
    
    rest_server->add_get_route(
        {"batch_id"}, [this](http_request request) {
            return this->batch_id_get_handler(request);
        });
}

void Super_Dealer::start() const{
    rest_server->wait_for_request();
}

http_response Super_Dealer::batch_id_get_handler(http_request request) {
    auto queryObjects = RestServer::queryObjects(request);
    // check if all required parameters are present
    if (queryObjects.find("model_id") == queryObjects.end()) {
      std::clog << "Model ID not found in query!\n";
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
    std::unique_ptr<PostgreSQL> postgresql;
    std::unique_ptr<MySQL> mysql;
    vector<vector<string>> batches_present;
    if (db_system == static_cast<int>(db_engine::mysql)){
      mysql = std::make_unique<MySQL>(sd_db_host, sd_db_user, sd_db_password, sd_database, sd_db_port);
      batches_present = mysql->parse_result(mysql->find_rows("batch", "model_id", model_id),
                                      super_dealer_batches_schema);
      
    }else if (db_system == static_cast<int>(db_engine::postgres)){
      postgresql = std::make_unique<PostgreSQL>(sd_db_host, sd_db_user, sd_db_password, sd_database, sd_db_port);
      batches_present = postgresql->parse_result(postgresql->find_rows(sd_database, "batch", "model_id", model_id),
                                      super_dealer_batches_schema);
    }else{
        http_response response(status_codes::BadRequest);
        stringstream reason;
        std::clog << "Value for parameter db can only be '0' or '1'\n";
        reason << "Value for parameter db can only be '0' or '1'";
        response.set_reason_phrase(reason.str());
        request.reply(status_codes::NotFound);
        return response;
    }
    if (batches_present.empty()) {
      // batch doesn't exist for model_id
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
    // Select unused batch_id from Super Dealer's batch table and update table.
    vector<int>::size_type total_batches_present = batches_present.size();
    string unused_batch_id;
    for(int i = 0; i < total_batches_present; i++){
        if(stoi(batches_present[i][2]) == 0){
            unused_batch_id = batches_present[i][1];
            if (db_system == static_cast<int>(db_engine::mysql)){
              mysql->update("batch", "batch_id", unused_batch_id, 
              super_dealer_batches_schema, {unused_batch_id, model_id, "1"});
            }else if (db_system == static_cast<int>(db_engine::postgres)){
              postgresql->update(sd_database, "batch", "batch_id", unused_batch_id, 
              super_dealer_batches_schema, {unused_batch_id, model_id, "1"});
            }
        }
    }
    // create json response
    json::value response_json;
    response_json["batch_id"] = json::value::string(unused_batch_id);

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
  opt::options_description desc("dealer converter command line options");
  int sd_id; 
  // clang-format off
  desc.add_options()
    ("id", opt::value<int>(&sd_id), "ID of Super Dealer")
    ("help", "display help message");
  // clang-format on
  // parse command line arguments
  opt::variables_map vm;
  opt::store(opt::parse_command_line(argc, argv, desc), vm);
  opt::notify(vm);
  if (!vm.count("id") || !check_valid_id(config, sd_id, "super_dealers")) {
    std::cout << "Invalid entry for ID! Usage Options: --id <id>";
    return 1;
  }
  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }
  YAML::Node sd_config = ConfigLoader::getSuperDealerConfig(config, sd_id);

  bool https = sd_config["https"].as<bool>();
  int db_system = sd_config["db_engine"].as<int>();
  string db_str = "mysql";
  if (db_system == static_cast<int>(db_engine::postgres)){
    db_str = "postgres";
  }
  auto sd_cert = sd_config["sd_cert"].as<string>();
  auto sd_privkey = sd_config["sd_privkey"].as<string>();
  auto sd_db_host = sd_config["sd_"+db_str+"_host"].as<string>();
  int sd_db_port = sd_config["sd_"+db_str+"_port"].as<int>();
  auto sd_db_user = sd_config["sd_"+db_str+"_user"].as<string>();
  auto sd_db_password = sd_config["sd_"+db_str+"_password"].as<string>();
  auto sd_database = sd_config["sd_"+db_str+"_database"].as<string>();

  auto sd_host = sd_config["sd_host"].as<string>();
  int sd_port = sd_config["sd_port"].as<int>();

  Super_Dealer sd(sd_db_host, sd_db_user, sd_db_password, sd_database, 
                 sd_db_port, sd_host, sd_port, https, db_system,
                 sd_cert, sd_privkey);
  sd.start();
  return 0;
}