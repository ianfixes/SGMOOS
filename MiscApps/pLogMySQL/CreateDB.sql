CREATE DATABASE IF NOT EXISTS moos;
USE moos;

CREATE TABLE IF NOT EXISTS mission (
   mission_id SMALLINT UNSIGNED NOT NULL AUTO_INCREMENT,
   date DATE,
   time TIME,
   vehicle_name VARCHAR(31),
   label VARCHAR(100),
   location VARCHAR(63),
   origin_latitude DOUBLE,
   origin_longitude DOUBLE,
   notes TEXT,
   PRIMARY KEY (mission_id)
)ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS app_data (
   mission_id SMALLINT UNSIGNED NOT NULL,
   elapsed_time DOUBLE NOT NULL,
   varname VARCHAR(100) NOT NULL,
   app VARCHAR(100),
   value DOUBLE,
   FOREIGN KEY (mission_id) REFERENCES mission(mission_id) ON DELETE CASCADE
)ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS app_messages (
   mission_id SMALLINT UNSIGNED NOT NULL,
   elapsed_time DOUBLE NOT NULL,
   varname VARCHAR(100) NOT NULL,
   app VARCHAR(100),
   message VARCHAR(255),
   FOREIGN KEY (mission_id) REFERENCES mission(mission_id) ON DELETE CASCADE
)ENGINE=InnoDB;

CREATE TABLE IF NOT EXISTS text_files (
   mission_id SMALLINT UNSIGNED NOT NULL,
   file_name VARCHAR(200) NOT NULL,
   file TEXT NOT NULL,
   PRIMARY KEY (mission_id, file_name),
   FOREIGN KEY (mission_id) REFERENCES mission(mission_id) ON DELETE CASCADE
)ENGINE=InnoDB;

GRANT INSERT ON moos.* TO 'mooslogger'@'localhost' IDENTIFIED BY 'moosloggerpass';
GRANT SELECT ON moos.* TO 'moosreader'@'%';

