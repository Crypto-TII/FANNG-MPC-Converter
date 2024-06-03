#include "db_schemas.h"

using std::string;
using std::vector;

const std::vector<std::string> dealer_shares_schema{"share", "mac_share"};
const std::vector<std::string> dealer_batches_schema{"batch_id", "model_id",
                                                     "mac_key_share", "shares_seed", "mac_key_mask"};
const std::vector<std::string> dealer_types_per_batch_schema{"batch_id", 
                                                    "material_type_id", "start_row", "end_row",
                                                     "order_in_batch"};

const std::vector<std::string> model_owner_shares_schema{"share",
                                                         "mac_share"};                                                        
const std::vector<std::string> model_owner_models_schema{"model_id",
                                                         "model_name"};
const std::vector<std::string> model_owner_types_per_model_schema{"model_id",
                                                         "num_materials", "material_type_id"};
const std::vector<std::string> model_owner_types_per_batch_schema{"batch_id",
                                                                "material_type_id", "start_row", "end_row",
                                                                "order_in_batch"};
const std::vector<std::string> model_owner_batches_schema{"batch_id",
                                                         "model_id", "mac_key_share", "shares_delivered"};
const std::vector<std::string> model_owner_material_type_schema{"material_type_id",
                                                         "material_type"};

const std::vector<std::string> client_shares_schema{"share", "mac_share"};
const std::vector<std::string> client_batches_schema{"batch_id", "model_id",
                                                     "mac_key_share"};
const std::vector<std::string> client_types_per_batch_schema{"batch_id", 
                                                    "material_type_id", "start_row", "end_row",
                                                     "order_in_batch"};
const std::vector<std::string> client_types_per_model_schema{"model_id",
                                                         "num_materials", "material_type_id"};
const std::vector<std::string> client_material_type_schema{"material_type_id",
                                                         "material_type"};

const std::vector<std::string> super_dealer_model_schema{"model_id",
                                                         "model_name"};
const std::vector<std::string> super_dealer_batches_schema{"batch_id",
                                                         "model_id", "used"};