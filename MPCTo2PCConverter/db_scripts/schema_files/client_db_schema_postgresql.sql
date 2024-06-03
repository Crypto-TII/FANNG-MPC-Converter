-- -----------------------------------------------------
-- Schema client
-- -----------------------------------------------------
DROP SCHEMA IF EXISTS "client" CASCADE;
-- -----------------------------------------------------
-- Schema client
-- -----------------------------------------------------
CREATE SCHEMA IF NOT EXISTS "client";
SET search_path = "client";
-- -----------------------------------------------------
-- Table "client"."model"
-- -----------------------------------------------------
DROP TABLE IF EXISTS "client"."model" CASCADE;
CREATE TABLE IF NOT EXISTS "client"."model" (
  "model_id" SERIAL PRIMARY KEY,
  "model_name" VARCHAR(50) NOT NULL
);
-- -----------------------------------------------------
-- Table "client"."batch"
-- -----------------------------------------------------
DROP TABLE IF EXISTS "client"."batch" CASCADE;
CREATE TABLE IF NOT EXISTS "client"."batch" (
  "batch_id" SERIAL PRIMARY KEY,
  "model_id" INT NOT NULL,
  "mac_key_share" VARCHAR(200) NOT NULL,
  CONSTRAINT "fk_batch_model" FOREIGN KEY ("model_id") REFERENCES "client"."model" ("model_id") ON DELETE NO ACTION ON UPDATE NO ACTION
);
-- -----------------------------------------------------
-- Table "client"."share"
-- -----------------------------------------------------
DROP TABLE IF EXISTS "client"."share" CASCADE;
CREATE TABLE IF NOT EXISTS "client"."share" (
  "share_id" SERIAL PRIMARY KEY,
  "share" VARCHAR(200) NOT NULL,
  "mac_share" VARCHAR(200) NOT NULL
);
-- -----------------------------------------------------
-- Table "client"."material_type"
-- -----------------------------------------------------
DROP TABLE IF EXISTS "client"."material_type" CASCADE;
CREATE TABLE IF NOT EXISTS "client"."material_type" (
  "material_type_id" SERIAL PRIMARY KEY,
  "material_type" VARCHAR(50) NOT NULL
);
-- -----------------------------------------------------
-- Table "client"."types_per_batch"
-- -----------------------------------------------------
DROP TABLE IF EXISTS "client"."types_per_batch" CASCADE;
CREATE TABLE IF NOT EXISTS "client"."types_per_batch" (
  "types_per_batch_id" SERIAL PRIMARY KEY,
  "batch_id" INT NOT NULL,
  "material_type_id" INT NOT NULL,
  "start_row" INT NOT NULL,
  "end_row" INT NOT NULL,
  "order_in_batch" INT NOT NULL,
  CONSTRAINT "fk_types_per_batch_batch" FOREIGN KEY ("batch_id") REFERENCES "client"."batch" ("batch_id") ON DELETE NO ACTION ON UPDATE NO ACTION,
  CONSTRAINT "fk_types_per_batch_material_type" FOREIGN KEY ("material_type_id") REFERENCES "client"."material_type" ("material_type_id") ON DELETE NO ACTION ON UPDATE NO ACTION
);
-- -----------------------------------------------------
-- Table "client"."types_per_model"
-- -----------------------------------------------------
DROP TABLE IF EXISTS "client"."types_per_model" CASCADE;
CREATE TABLE IF NOT EXISTS "client"."types_per_model" (
  "types_per_model_id" SERIAL PRIMARY KEY,
  "model_id" INT NOT NULL,
  "material_type_id" INT NOT NULL,
  "num_materials" INT NOT NULL,
  CONSTRAINT "fk_types_per_model_model" FOREIGN KEY ("model_id") REFERENCES "client"."model" ("model_id") ON DELETE NO ACTION ON UPDATE NO ACTION
);
