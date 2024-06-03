-- -----------------------------------------------------
-- Schema super_dealer
-- -----------------------------------------------------
DROP SCHEMA IF EXISTS "super_dealer" CASCADE;
CREATE SCHEMA IF NOT EXISTS "super_dealer";
SET search_path = "super_dealer";
-- -----------------------------------------------------
-- Table "super_dealer"."model"
-- -----------------------------------------------------
DROP TABLE IF EXISTS "super_dealer"."model";
CREATE TABLE IF NOT EXISTS "super_dealer"."model" (
  "model_id" SERIAL PRIMARY KEY,
  "model_name" VARCHAR(50) NOT NULL
);
-- -----------------------------------------------------
-- Table "super_dealer"."batch"
-- -----------------------------------------------------
DROP TABLE IF EXISTS "super_dealer"."batch";
CREATE TABLE IF NOT EXISTS "super_dealer"."batch" (
  "batch_id" SERIAL PRIMARY KEY,
  "model_id" INT NOT NULL,
  "used" SMALLINT NOT NULL,
  CONSTRAINT "fk_batch_model"
    FOREIGN KEY ("model_id")
    REFERENCES "super_dealer"."model" ("model_id")
);
