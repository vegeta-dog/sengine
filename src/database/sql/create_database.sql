-- MySQL Workbench Forward Engineering

SET @OLD_UNIQUE_CHECKS=@@UNIQUE_CHECKS, UNIQUE_CHECKS=0;
SET @OLD_FOREIGN_KEY_CHECKS=@@FOREIGN_KEY_CHECKS, FOREIGN_KEY_CHECKS=0;
SET @OLD_SQL_MODE=@@SQL_MODE, SQL_MODE='ONLY_FULL_GROUP_BY,STRICT_TRANS_TABLES,NO_ZERO_IN_DATE,NO_ZERO_DATE,ERROR_FOR_DIVISION_BY_ZERO,NO_ENGINE_SUBSTITUTION';

-- -----------------------------------------------------
-- Schema sEngine
-- -----------------------------------------------------

-- -----------------------------------------------------
-- Schema sEngine
-- -----------------------------------------------------
CREATE SCHEMA IF NOT EXISTS `sEngine` ;
USE `sEngine` ;

-- -----------------------------------------------------
-- Table `sEngine`.`WebPage`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `sEngine`.`WebPage` (
  `idWebPage` INT NOT NULL AUTO_INCREMENT,
  `url` INT NOT NULL,
  `title` TEXT(50) NULL,
  `document` TEXT(1200) NULL,
  `UpdatedTime` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  `count_total_link_to` INT NOT NULL DEFAULT 0,
  `Crawl` INT NOT NULL DEFAULT 0,
  `CreatedDateTime` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
  PRIMARY KEY (`idWebPage`),
  UNIQUE INDEX `idWebPage_UNIQUE` (`idWebPage` ASC) VISIBLE,
  UNIQUE INDEX `url_UNIQUE` (`url` ASC) VISIBLE)
ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `sEngine`.`LinkTable`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `sEngine`.`LinkTable` (
  `From` INT NOT NULL,
  `To` INT NOT NULL,
  `UpdatedTime` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  INDEX `From_idx` (`From` ASC) VISIBLE,
  INDEX `To_idx` (`To` ASC) VISIBLE,
  CONSTRAINT `From`
    FOREIGN KEY (`From`)
    REFERENCES `sEngine`.`WebPage` (`idWebPage`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION,
  CONSTRAINT `To`
    FOREIGN KEY (`To`)
    REFERENCES `sEngine`.`WebPage` (`idWebPage`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
ENGINE = InnoDB
DEFAULT CHARACTER SET = binary;


-- -----------------------------------------------------
-- Table `sEngine`.`InvertedIndexTable`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `sEngine`.`InvertedIndexTable` (
  `idInvertedIndexTable` INT NOT NULL AUTO_INCREMENT,
  `key` VARCHAR(25) NOT NULL,
  `path` VARCHAR(100) NOT NULL,
  `UpdatedDatetime` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
  PRIMARY KEY (`idInvertedIndexTable`),
  INDEX `key` (`key` ASC) VISIBLE,
  UNIQUE INDEX `key_UNIQUE` (`key` ASC) VISIBLE)
ENGINE = InnoDB
COMMENT = '倒排索引表';


SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
