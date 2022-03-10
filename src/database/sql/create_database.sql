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
-- Table `sEngine`.`WebLink`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `sEngine`.`WebLink` (
                                                   `idWebLink` INT NOT NULL AUTO_INCREMENT,
                                                   `url` TEXT(200) NULL,
    `Crawl` TINYINT NOT NULL DEFAULT 0,
    `CreatedDateTime` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    `UpdatedDateTime` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    PRIMARY KEY (`idWebLink`))
    ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `sEngine`.`WebPage`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `sEngine`.`WebPage` (
                                                   `idWebPage` INT NOT NULL AUTO_INCREMENT,
                                                   `url` INT NOT NULL,
                                                   `title` TEXT(50) NOT NULL,
    `document` TEXT(1200) NOT NULL,
    `UpdatedTime` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP,
    `count_total_link_to` INT NULL,
    PRIMARY KEY (`idWebPage`),
    UNIQUE INDEX `idWebPage_UNIQUE` (`idWebPage` ASC) VISIBLE,
    UNIQUE INDEX `url_UNIQUE` (`url` ASC) VISIBLE,
    CONSTRAINT `toWebLink`
    FOREIGN KEY (`url`)
    REFERENCES `sEngine`.`WebLink` (`idWebLink`)
                                                              ON DELETE RESTRICT
                                                              ON UPDATE NO ACTION)
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
    ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `sEngine`.`KeyTable`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `sEngine`.`KeyTable` (
                                                    `idKeyTable` INT NOT NULL AUTO_INCREMENT,
                                                    `key` VARCHAR(25) NOT NULL,
    `search_count` INT NOT NULL DEFAULT 0,
    PRIMARY KEY (`idKeyTable`),
    UNIQUE INDEX `key_UNIQUE` (`key` ASC) INVISIBLE,
    INDEX `key` (`key` ASC) VISIBLE)
    ENGINE = InnoDB;


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
    UNIQUE INDEX `key_UNIQUE` (`key` ASC) VISIBLE,
    CONSTRAINT `to_keyTable`
    FOREIGN KEY (`key`)
    REFERENCES `sEngine`.`KeyTable` (`key`)
                                                                  ON DELETE RESTRICT
                                                                  ON UPDATE NO ACTION)
    ENGINE = InnoDB
    COMMENT = '倒排索引表';


-- -----------------------------------------------------
-- Table `sEngine`.`SearchRecord`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `sEngine`.`SearchRecord` (
                                                        `idSearchRecord` INT NOT NULL,
                                                        `RawData` VARCHAR(45) NOT NULL,
    `SearchDateTime` DATETIME NOT NULL,
    `CreatedDateTime` DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP,
    PRIMARY KEY (`idSearchRecord`))
    ENGINE = InnoDB;


-- -----------------------------------------------------
-- Table `sEngine`.`KeyLink`
-- -----------------------------------------------------
CREATE TABLE IF NOT EXISTS `sEngine`.`KeyLink` (
                                                   `toSearchRecord` INT NOT NULL,
                                                   `toKey` VARCHAR(25) NOT NULL,
    INDEX `toRec_idx` (`toSearchRecord` ASC) VISIBLE,
    INDEX `to_Key_idx` (`toKey` ASC) VISIBLE,
    CONSTRAINT `to_Rec`
    FOREIGN KEY (`toSearchRecord`)
    REFERENCES `sEngine`.`SearchRecord` (`idSearchRecord`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION,
    CONSTRAINT `to_Key`
    FOREIGN KEY (`toKey`)
    REFERENCES `sEngine`.`KeyTable` (`key`)
    ON DELETE CASCADE
    ON UPDATE NO ACTION)
    ENGINE = InnoDB;


SET SQL_MODE=@OLD_SQL_MODE;
SET FOREIGN_KEY_CHECKS=@OLD_FOREIGN_KEY_CHECKS;
SET UNIQUE_CHECKS=@OLD_UNIQUE_CHECKS;
