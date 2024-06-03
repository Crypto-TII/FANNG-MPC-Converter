#include "client.h"

namespace opt = boost::program_options;

using std::make_shared;
using std::shared_ptr;
using std::string;
using std::stringstream;
using std::to_string;
using std::vector;
using std::tuple;

Client::Client(const string &client_db_host, const string &client_db_user,
               const string &client_db_password, const string &client_database,
               const int client_db_port, bool https, const int &db_system,
               const string &mo_host, const int &mo_port, const string &mo_cert,
               const vector<string> &dealer_rest_hosts, const vector<string> &dealer_rest_certs,
              const vector<int> &dealer_rest_ports)
    : client_db_host(client_db_host), client_db_user(client_db_user), client_db_password(client_db_password),
      client_database(client_database), client_db_port(client_db_port), https(https), db_system(db_system),
      mo_host(mo_host), mo_port(mo_port), mo_cert(mo_cert), dealer_rest_hosts(dealer_rest_hosts),
      dealer_rest_certs(dealer_rest_certs), dealer_rest_ports(dealer_rest_ports) {}

std::tuple<std::string, int> Client::get_batch_id(const int& model_id) {
    shared_ptr<RestClient> clientOfModelOwner;
    string batch_id;
    if (https) {
      clientOfModelOwner = make_shared<RestHTTPSClient>(mo_host, mo_port, mo_cert);
    } else {
      clientOfModelOwner = make_shared<RestClient>(mo_host, mo_port);
    }
    clientOfModelOwner->make_request(web::http::methods::GET,
                          web::http::uri_builder()
                          .append_path(utility::string_t("/model_inference"))
                          .append_query(utility::string_t("model_id"), model_id),
                          [&batch_id](web::json::value v) {
                            if (v.has_field("batch_id")) {
                              batch_id = v[utility::string_t("batch_id")].as_string();
                            }
                          });
 

    if (batch_id.empty()) {
      std::cout << "Error: could not retrieve model information" << std::endl;
      return {};
    }else{
      std::cout << "Received batch_id = " << batch_id << " from Model Owner for model_id = " << model_id <<std::endl;
    }
    std::unique_ptr<PostgreSQL> postgresql;
    std::unique_ptr<MySQL> mysql;
    // Fill Client's types_per_batch table
    int total_shares_per_batch = 0;
    int row = 1;
    int order_in_batch = 1;
    vector<vector<string>> types_per_model_rows;
    vector<vector<string>> material_type_rows;
    if (db_system == static_cast<int>(db_engine::mysql)){
      mysql = std::make_unique<MySQL>(client_db_host, client_db_user, client_db_password, client_database, client_db_port);
      mysql->insert_one("batch", client_batches_schema, {batch_id,  to_string(model_id), "-1"});
      types_per_model_rows = mysql->parse_result(mysql->find_rows("types_per_model", "model_id", 
                              to_string(model_id)), client_types_per_model_schema); 
      for(auto& rows: types_per_model_rows){
        string material_type_id = rows[2];
        material_type_rows = mysql->parse_result(mysql->find_rows("material_type", "material_type_id", 
                              material_type_id), client_material_type_schema); 
        string material_name = material_type_rows[0][1];
        int num_materials = get_material_size(material_name, stoi(rows[1]));
        total_shares_per_batch += num_materials; 
        mysql->insert_one("types_per_batch", client_types_per_batch_schema, 
                  {batch_id, material_type_id,
                  to_string(row), to_string(row+num_materials), to_string(order_in_batch)});
        row += num_materials;
        order_in_batch++;
      }
    }else if (db_system == static_cast<int>(db_engine::postgres)){
      postgresql = std::make_unique<PostgreSQL>(client_db_host, client_db_user, client_db_password, client_database, client_db_port);
      postgresql->insert_one(client_database, "batch", client_batches_schema,
                {batch_id,  to_string(model_id), "-1"});
      types_per_model_rows = postgresql->parse_result(postgresql->find_rows(client_database, "types_per_model", "model_id", 
                            to_string(model_id)), client_types_per_model_schema);   
      for(auto& rows: types_per_model_rows){
        string material_type_id = rows[2];
        material_type_rows = postgresql->parse_result(postgresql->find_rows(client_database, "material_type", "material_type_id", 
                              material_type_id), client_material_type_schema); 
        string material_name = material_type_rows[0][1];
        int num_materials = get_material_size(material_name, stoi(rows[1]));
        total_shares_per_batch += num_materials; 
        postgresql->insert_one(client_database, "types_per_batch", client_types_per_batch_schema, 
                  {batch_id, material_type_id,
                  to_string(row), to_string(row+num_materials), to_string(order_in_batch)});
        row += num_materials;
        order_in_batch++;
      } 
    }
    std::cout << "Done with filling client's types_per_batch table!" << std::endl;
    return {batch_id, total_shares_per_batch};
}

tuple<vector<string>, vector<string>> Client::get_shares_seed_and_mac_mask(const string& batch_id, const vector<int>& dealer_indices) {
    size_t num_dealers = dealer_indices.size();
    vector<string> seeds(num_dealers);
    vector<string> mac_masks(num_dealers);
    // connect to dealers to get seed
    std::cout << "Making request to dealers for seeds!" << std::endl;
    for (int i = 0; i < dealer_indices.size(); i++) {
      shared_ptr<RestClient> clientOfDealer;
      if (https) {
        clientOfDealer = make_shared<RestHTTPSClient>(dealer_rest_hosts[dealer_indices[i]], dealer_rest_ports[dealer_indices[i]],
                                              dealer_rest_certs[dealer_indices[i]]);
      } else {
        clientOfDealer = make_shared<RestClient>(dealer_rest_hosts[dealer_indices[i]], dealer_rest_ports[dealer_indices[i]]);
      }
      bool poll = true;
      while (poll) {
        poll = false;
        std::clog << "getting seed from dealer " << dealer_indices[i] << std::endl;
        clientOfDealer->make_request(
            web::http::methods::GET,
            web::http::uri_builder()
                .append_path(utility::string_t("/seed"))
                .append_query(utility::string_t("batch_id"), batch_id),
            [&poll, i, &seeds, &mac_masks, &dealer_indices, &batch_id](web::json::value v) {
              if (!v.has_field("shares_seed") || !v.has_field("mac_key_mask")) {
                std::clog << "dealer " << dealer_indices[i] << " has no seed for batch "
                          << batch_id << ". waiting .... " << std::endl;
                // wait for 5 seconds
                std::this_thread::sleep_for(std::chrono::seconds(5));
                poll = true;
                return;
              }
              seeds[i] = v[utility::string_t("shares_seed")].as_string();
              mac_masks[i] = v[utility::string_t("mac_key_mask")].as_string();
            });
      }
    }
    if(seeds.empty()){
      return {};
    }
    return {seeds, mac_masks};

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
  int client_id; // Default as we only have one client

  opt::options_description desc("dealer converter command line options");
  desc.add_options()
    ("id", opt::value<int>(&client_id), "ID of client")
    ("help", "display help message");

  // clang-format on
  // parse command line arguments
  opt::variables_map vm;
  opt::store(opt::parse_command_line(argc, argv, desc), vm);
  opt::notify(vm);
  if (!vm.count("id") || !check_valid_id(config, client_id, "clients")) {
    std::cout << "Invalid entry for ID! Usage Options: --id <id>";
    return 1;
  }
  if (vm.count("help")) {
    std::cout << desc << "\n";
    return 1;
  }
  YAML::Node client_config = ConfigLoader::getClientConfig(config, client_id);
  // Check if command line options are provided, if not, load from config
  bool https = client_config["https"].as<bool>();
  int db_system = client_config["db_engine"].as<int>();
  int model_id = client_config["model_id"].as<int>();
  string dealer_ids_str = client_config["dealer_ids_str"].as<string>();

  auto mo_host = client_config["mo_host"].as<string>();
  int mo_port = client_config["mo_port"].as<int>();
  auto mo_cert = client_config["mo_cert"].as<string>();
  string db_str = "mysql";
  if (db_system == static_cast<int>(db_engine::postgres)){
    db_str = "postgres";
  }
  auto client_db_host = client_config["client_"+db_str+"_host"].as<string>();
  int client_db_port = client_config["client_"+db_str+"_port"].as<int>();
  auto client_db_user = client_config["client_"+db_str+"_user"].as<string>();
  auto client_db_password = client_config["client_"+db_str+"_password"].as<string>();
  auto client_database = client_config["client_"+db_str+"_database"].as<string>();
  
  int max_share_size = client_config["max_share_size"].as<int>();
  size_t num_dealers = client_config["num_dealers"].as<int>();

  vector<string> dealer_rest_hosts;
  vector<int> dealer_rest_ports;
  vector<string> dealer_rest_certs;
  for(int i = 0; i < num_dealers; i++){
    YAML::Node dealer_config = ConfigLoader::getDealerConfig(config, i);
    dealer_rest_hosts.emplace_back(dealer_config["rest_host"].as<string>());
    dealer_rest_certs.emplace_back(dealer_config["rest_cert"].as<string>());
    dealer_rest_ports.push_back(dealer_config["rest_port"].as<int>());
  }
  // connect to model owner to get batch id
  Client client(client_db_host, client_db_user, client_db_password, client_database,
              client_db_port, https, db_system, mo_host, mo_port, mo_cert, dealer_rest_hosts,
              dealer_rest_certs, dealer_rest_ports);
  // batch_id request to model owner
  std::cout << "Requesting Model Owner for batch_id!" << std::endl;
  string batch_id;
  int total_shares_per_batch;
  std::tie(batch_id, total_shares_per_batch) = client.get_batch_id(model_id);
  // seed request to dealers
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
  num_dealers = dealer_indices.size();
  vector<string> seeds(num_dealers);
  vector<string> mac_masks(num_dealers);
  std::cout << "Requesting Dealers for shares seed and mac key mask!" << std::endl;
  std::tie(seeds, mac_masks) = client.get_shares_seed_and_mac_mask(batch_id, dealer_indices);
  std::cout << "Received from Dealers! \nshares_seed size: " << seeds.size() << ", mac seed size: " << mac_masks.size() << std::endl;
  std::clog << "generate 2PC shares and mac_key_share" << std::endl;
  // write 2PC shares to local database
  vector<shared_ptr<AES_RNG>> prgs(num_dealers);
  TiiPrime mac_key_share2pc;
  for (int i = 0; i < num_dealers; i++) {
    // generating prg
    CryptoPP::SecByteBlock sk = string2seed(seeds[i]);
    prgs[i] = generate_prg(sk);

    // demasking mac_key_share
    TiiPrime x;
    stringstream ss(mac_masks[i]);
    ss >> x;
    mac_key_share2pc += x;
  } 
  // demask share, mac_share
  vector<TiiPrime> shares2pc(total_shares_per_batch);
  vector<TiiPrime> mac_shares2pc(total_shares_per_batch);
  CryptoPP::SecByteBlock masks(field_size * total_shares_per_batch);
  const __int128 *masks_ptr;
  auto gen_2pc = [&masks, &masks_ptr, &total_shares_per_batch](shared_ptr<AES_RNG> prg,
                                            vector<TiiPrime> &shares_2pc) {
    prg->GenerateBlock(masks, masks.size());
    masks_ptr = reinterpret_cast<const __int128 *>(masks.data());
    for (int j = 0; j < total_shares_per_batch; j++) {
      TiiPrime x(ntoh128(masks_ptr[j]), true);
      shares_2pc[j] += x;
    }
  };

  for (int i = 0; i < num_dealers; i++) {
    gen_2pc(prgs[i], shares2pc);
    gen_2pc(prgs[i], mac_shares2pc);
  }
  // write 2PC shares to local database
  std::unique_ptr<PostgreSQL> postgresql;
  std::unique_ptr<MySQL> mysql;
  // Update client's batch table
  if (db_system == static_cast<int>(db_engine::mysql)){
    mysql = std::make_unique<MySQL>(client_db_host, client_db_user, client_db_password, client_database, client_db_port);  
    mysql->update("batch", "batch_id", batch_id, client_batches_schema, 
                {batch_id, to_string(model_id), mac_key_share2pc.to_string()});
  }else if (db_system == static_cast<int>(db_engine::postgres)){
    postgresql = std::make_unique<PostgreSQL>(client_db_host, client_db_user, client_db_password, client_database, client_db_port);  
    postgresql->update(client_database, "batch", "batch_id", batch_id, client_batches_schema, 
                {batch_id, to_string(model_id), mac_key_share2pc.to_string()});
  }
  // batching strategy when storing shares in client's side
  std::clog << "writing 2PC shares to client's database" << std::endl;
  for (size_t i = 0; i < shares2pc.size(); i += max_share_size) {
      size_t end = std::min(i + max_share_size, shares2pc.size());
      vector<TiiPrime> batch_2pc_shares(shares2pc.begin() + i, shares2pc.begin() + end);
      vector<TiiPrime> batch_2pc_mac_shares(mac_shares2pc.begin() + i, mac_shares2pc.begin() + end);
      vector<vector<string>> shares_and_mac_shares;
      for(int j = 0; j < total_shares_per_batch; j++){
        shares_and_mac_shares.push_back({batch_2pc_shares[j].to_string(), batch_2pc_mac_shares[j].to_string()});
      }
      if (db_system == static_cast<int>(db_engine::mysql)){
        mysql->insert_many("share", client_shares_schema, shares_and_mac_shares);
      }else if (db_system == static_cast<int>(db_engine::postgres)){
        postgresql->insert_many(client_database, "share", client_shares_schema, shares_and_mac_shares);
      }
    }
  std::cout << "Done filling 2pc shares to client's db!" << std::endl;
  return 0;
}
