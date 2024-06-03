#ifndef __DB_SCHEMAS_H__
#define __DB_SCHEMAS_H__

#include <vector>
#include <string>

extern const std::vector<std::string> dealer_shares_schema;
extern const std::vector<std::string> dealer_batches_schema;
extern const std::vector<std::string> dealer_types_per_batch_schema;

extern const std::vector<std::string> model_owner_shares_schema;
extern const std::vector<std::string> model_owner_batches_schema;
extern const std::vector<std::string> model_owner_types_per_model_schema;
extern const std::vector<std::string> model_owner_types_per_batch_schema;
extern const std::vector<std::string> model_owner_models_schema;
extern const std::vector<std::string> model_owner_material_type_schema;

extern const std::vector<std::string> client_shares_schema;
extern const std::vector<std::string> client_batches_schema;
extern const std::vector<std::string> client_types_per_batch_schema;
extern const std::vector<std::string> client_types_per_model_schema;
extern const std::vector<std::string> client_material_type_schema;

extern const std::vector<std::string> super_dealer_model_schema;
extern const std::vector<std::string> super_dealer_batches_schema;

#endif //__DB_SCHEMAS_H__