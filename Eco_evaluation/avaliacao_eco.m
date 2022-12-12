% Reset everything
clear; clc;

% Opening serial conection
s = serialport("/dev/ttyACM0", 9600);


enviado = read(s,1,"uint8")
recebido = read(s,5,"uint8")