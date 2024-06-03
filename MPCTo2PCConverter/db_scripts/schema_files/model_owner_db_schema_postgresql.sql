-- -----------------------------------------------------
-- Schema model_owner
-- -----------------------------------------------------
DROP SCHEMA IF EXISTS "model_owner" CASCADE;
-- -----------------------------------------------------
-- Schema model_owner
-- -----------------------------------------------------
CREATE SCHEMA IF NOT EXISTS "model_owner";
SET search_path = "model_owner";
-- -----------------------------------------------------
-- Table "model_owner"."model"
-- -----------------------------------------------------
DROP TABLE IF EXISTS "model_owner"."model" CASCADE;
CREATE TABLE IF NOT EXISTS "model_owner"."model" (
  "model_id" SERIAL PRIMARY KEY,
  "model_name" VARCHAR(50) NOT NULL
);
-- -----------------------------------------------------
-- Table "model_owner"."share"
-- -----------------------------------------------------
DROP TABLE IF EXISTS "model_owner"."share" CASCADE;
CREATE TABLE IF NOT EXISTS "model_owner"."share" (
  "share_id" SERIAL PRIMARY KEY,
  "share" VARCHAR(200) NOT NULL,
  "mac_share" VARCHAR(200) NOT NULL
);
-- -----------------------------------------------------
-- Table "model_owner"."batch"
-- -----------------------------------------------------
DROP TABLE IF EXISTS "model_owner"."batch" CASCADE;
CREATE TABLE IF NOT EXISTS "model_owner"."batch" (
  "batch_id" SERIAL PRIMARY KEY,
  "model_id" INT NOT NULL,
  "mac_key_share" VARCHAR(200) NOT NULL,
  "shares_delivered" SMALLINT NOT NULL,
  CONSTRAINT "fk_batch_model" FOREIGN KEY ("model_id") REFERENCES "model_owner"."model" ("model_id") ON DELETE NO ACTION ON UPDATE NO ACTION
);
-- -----------------------------------------------------
-- Table "model_owner"."material_type"
-- -----------------------------------------------------
DROP TABLE IF EXISTS "model_owner"."material_type" CASCADE;
CREATE TABLE IF NOT EXISTS "model_owner"."material_type" (
  "material_type_id" SERIAL PRIMARY KEY,
  "material_type" VARCHAR(50) NOT NULL
);
-- -----------------------------------------------------
-- Table "model_owner"."types_per_batch"
-- -----------------------------------------------------
DROP TABLE IF EXISTS "model_owner"."types_per_batch" CASCADE;
CREATE TABLE IF NOT EXISTS "model_owner"."types_per_batch" (
  "types_per_batch_id" SERIAL PRIMARY KEY,
  "batch_id" INT NOT NULL,
  "material_type_id" INT NOT NULL,
  "start_row" INT NOT NULL,
  "end_row" INT NOT NULL,
  "order_in_batch" INT NOT NULL,
  CONSTRAINT "fk_types_per_batch_batch" FOREIGN KEY ("batch_id") REFERENCES "model_owner"."batch" ("batch_id") ON DELETE NO ACTION ON UPDATE NO ACTION,
  CONSTRAINT "fk_types_per_batch_material_type" FOREIGN KEY ("material_type_id") REFERENCES "model_owner"."material_type" ("material_type_id") ON DELETE NO ACTION ON UPDATE NO ACTION
);
-- -----------------------------------------------------
-- Table "model_owner"."types_per_model"
-- -----------------------------------------------------
DROP TABLE IF EXISTS "model_owner"."types_per_model" CASCADE;
CREATE TABLE IF NOT EXISTS "model_owner"."types_per_model" (
  "types_per_model_id" SERIAL PRIMARY KEY,
  "model_id" INT NOT NULL,
  "num_materials" INT NOT NULL,
  "material_type_id" INT NOT NULL,
  CONSTRAINT "fk_types_per_model_model" FOREIGN KEY ("model_id") REFERENCES "model_owner"."model" ("model_id") ON DELETE NO ACTION ON UPDATE NO ACTION,
  CONSTRAINT "fk_types_per_model_material_type" FOREIGN KEY ("material_type_id") REFERENCES "model_owner"."material_type" ("material_type_id") ON DELETE NO ACTION ON UPDATE NO ACTION
);
