CREATE DATABASE smarthouse;

USE smarthouse;

CREATE TABLE output (
  'id_output' INT PRIMARY KEY NOT NULL AUTO_INCREMENT,
  'status' int(3) NOT NULL
);

INSERT INTO `output` (`id_output`, `status`) VALUES
	(1, 0),
	(2, 0),
	(3, 0);

CREATE TABLE 'temperatura' (
  'id_sensor' INT PRIMARY KEY NOT NULL AUTO_INCREMENT,
  'data' date DEFAULT NULL,
  'hora' time DEFAULT NULL,
  'data_hora' timestamp NOT NULL DEFAULT current_timestamp(),
  'temperatura' float DEFAULT NULL,
  'umidade' float DEFAULT NULL,
  'indice_calor' float DEFAULT NULL
);