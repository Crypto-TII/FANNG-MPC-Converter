-- -----------------------------------------------------
-- Schema dealer
-- -----------------------------------------------------
DROP SCHEMA IF EXISTS "dealer" CASCADE;
-- -----------------------------------------------------
-- Schema dealer
-- -----------------------------------------------------
CREATE SCHEMA IF NOT EXISTS "dealer";
SET search_path = "dealer";
-- -----------------------------------------------------
-- Table "dealer"."model"
-- -----------------------------------------------------
DROP TABLE IF EXISTS "dealer"."model" CASCADE;
CREATE TABLE IF NOT EXISTS "dealer"."model" (
  "model_id" SERIAL PRIMARY KEY,
  "model_name" VARCHAR(50) NOT NULL
);
-- -----------------------------------------------------
-- Table "dealer"."batch"
-- -----------------------------------------------------
DROP TABLE IF EXISTS "dealer"."batch" CASCADE;
CREATE TABLE IF NOT EXISTS "dealer"."batch" (
  "batch_id" SERIAL PRIMARY KEY,
  "model_id" INT NOT NULL,
  "mac_key_share" VARCHAR(200) NOT NULL,
  "shares_seed" VARCHAR(200) NOT NULL,
  "mac_key_mask" VARCHAR(200) NOT NULL,
  CONSTRAINT "fk_batch_model" FOREIGN KEY ("model_id") REFERENCES "dealer"."model" ("model_id") ON DELETE NO ACTION ON UPDATE NO ACTION
);
-- -----------------------------------------------------
-- Table "dealer"."share"
-- -----------------------------------------------------
DROP TABLE IF EXISTS "dealer"."share" CASCADE;
CREATE TABLE IF NOT EXISTS "dealer"."share" (
  "share_id" SERIAL PRIMARY KEY,
  "share" VARCHAR(200) NOT NULL,
  "mac_share" VARCHAR(200) NOT NULL
);
-- -----------------------------------------------------
-- Table "dealer"."material_type"
-- -----------------------------------------------------
DROP TABLE IF EXISTS "dealer"."material_type" CASCADE;
CREATE TABLE IF NOT EXISTS "dealer"."material_type" (
  "material_type_id" SERIAL PRIMARY KEY,
  "material_type" VARCHAR(50) NOT NULL
);
-- -----------------------------------------------------
-- Table "dealer"."types_per_batch"
-- -----------------------------------------------------
DROP TABLE IF EXISTS "dealer"."types_per_batch" CASCADE;
CREATE TABLE IF NOT EXISTS "dealer"."types_per_batch" (
  "types_per_batch_id" SERIAL PRIMARY KEY,
  "batch_id" INT NOT NULL,
  "material_type_id" INT NOT NULL,
  "start_row" INT NOT NULL,
  "end_row" INT NOT NULL,
  "order_in_batch" INT NOT NULL,
  CONSTRAINT "fk_types_per_batch_batch" FOREIGN KEY ("batch_id") REFERENCES "dealer"."batch" ("batch_id") ON DELETE NO ACTION ON UPDATE NO ACTION,
  CONSTRAINT "fk_types_per_batch_material_type" FOREIGN KEY ("material_type_id") REFERENCES "dealer"."material_type" ("material_type_id") ON DELETE NO ACTION ON UPDATE NO ACTION
);
-- -----------------------------------------------------
-- Table "dealer"."types_per_model"
-- -----------------------------------------------------
DROP TABLE IF EXISTS "dealer"."types_per_model" CASCADE;
CREATE TABLE IF NOT EXISTS "dealer"."types_per_model" (
  "types_per_model_id" SERIAL PRIMARY KEY,
  "model_id" INT NOT NULL,
  "material_type_id" INT NOT NULL,
  "num_materials" INT NOT NULL,
  CONSTRAINT "fk_types_per_model_model" FOREIGN KEY ("model_id") REFERENCES "dealer"."model" ("model_id") ON DELETE NO ACTION ON UPDATE NO ACTION
);
