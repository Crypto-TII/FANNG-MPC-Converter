-- -----------------------------------------------------
-- Schema model_owner
-- -----------------------------------------------------
DROP SCHEMA IF EXISTS `model_owner` ;
-- -----------------------------------------------------
-- Schema model_owner
-- -----------------------------------------------------
CREATE SCHEMA IF NOT EXISTS `model_owner` ;
USE `model_owner` ;
-- -----------------------------------------------------
-- Table `model_owner`.`model`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `model_owner`.`model` ;
CREATE TABLE IF NOT EXISTS `model_owner`.`model` (
  `model_id` INT NOT NULL AUTO_INCREMENT,
  `model_name` VARCHAR(50) NOT NULL,
  PRIMARY KEY (`model_id`));
-- -----------------------------------------------------
-- Table `model_owner`.`share`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `model_owner`.`share` ;
CREATE TABLE IF NOT EXISTS `model_owner`.`share` (
  `share_id` INT NOT NULL AUTO_INCREMENT,
  `share` VARCHAR(200) NOT NULL,
  `mac_share` VARCHAR(200) NOT NULL,
  PRIMARY KEY (`share_id`));
-- -----------------------------------------------------
-- Table `model_owner`.`batch`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `model_owner`.`batch` ;
CREATE TABLE IF NOT EXISTS `model_owner`.`batch` (
  `batch_id` INT NOT NULL AUTO_INCREMENT,
  `model_id` INT NOT NULL,
  `mac_key_share` VARCHAR(200) NOT NULL,
  `shares_delivered` TINYINT NOT NULL,
  PRIMARY KEY (`batch_id`),
  INDEX `fk_batch_model_idx` (`model_id` ASC) VISIBLE,
  CONSTRAINT `fk_batch_model`
    FOREIGN KEY (`model_id`)
    REFERENCES `model_owner`.`model` (`model_id`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION);
-- -----------------------------------------------------
-- Table `model_owner`.`material_type`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `model_owner`.`material_type` ;
CREATE TABLE IF NOT EXISTS `model_owner`.`material_type` (
  `material_type_id` INT NOT NULL AUTO_INCREMENT,
  `material_type` VARCHAR(50) NOT NULL,
  PRIMARY KEY (`material_type_id`));
-- -----------------------------------------------------
-- Table `model_owner`.`types_per_batch`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `model_owner`.`types_per_batch` ;
CREATE TABLE IF NOT EXISTS `model_owner`.`types_per_batch` (
  `types_per_batch_id` INT NOT NULL AUTO_INCREMENT,
  `batch_id` INT NOT NULL,
  `material_type_id` INT NOT NULL,
  `start_row` INT NOT NULL,
  `end_row` INT NOT NULL,
  `order_in_batch` INT NOT NULL,
  PRIMARY KEY (`types_per_batch_id`),
  INDEX `fk_types_per_batch_batch_idx` (`batch_id` ASC) VISIBLE,
  INDEX `fk_types_per_batch_material_type_idx` (`material_type_id` ASC) VISIBLE,
  CONSTRAINT `fk_types_per_batch_batch`
    FOREIGN KEY (`batch_id`)
    REFERENCES `model_owner`.`batch` (`batch_id`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_types_per_batch_material_type`
    FOREIGN KEY (`material_type_id`)
    REFERENCES `model_owner`.`material_type` (`material_type_id`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION);
-- -----------------------------------------------------
-- Table `model_owner`.`types_per_model`
-- -----------------------------------------------------
DROP TABLE IF EXISTS `model_owner`.`types_per_model` ;
CREATE TABLE IF NOT EXISTS `model_owner`.`types_per_model` (
  `types_per_model_id` INT NOT NULL AUTO_INCREMENT,
  `model_id` INT NOT NULL,
  `num_materials` INT NOT NULL,
  `material_type_id` INT NOT NULL,
  PRIMARY KEY (`types_per_model_id`),
  INDEX `fk_types_per_model_model_idx` (`model_id` ASC) VISIBLE,
  UNIQUE INDEX `id_UNIQUE` (`types_per_model_id` ASC) VISIBLE,
  INDEX `fk_types_per_model_material_type_idx` (`material_type_id` ASC) VISIBLE,
  CONSTRAINT `fk_types_per_model_model`
    FOREIGN KEY (`model_id`)
    REFERENCES `model_owner`.`model` (`model_id`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION,
  CONSTRAINT `fk_types_per_model_material_type`
    FOREIGN KEY (`material_type_id`)
    REFERENCES `model_owner`.`material_type` (`material_type_id`)
    ON DELETE NO ACTION
    ON UPDATE NO ACTION);