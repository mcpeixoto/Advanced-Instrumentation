% Reset everything
clear; clc;

% Opening serial conection
s = serialport("/dev/ttyACM0", 9600);


enviado = read(s,1,"uint8")
answer = enviado * 2;
write(s, answer, "char");
recebido = read(s,5,"uint8")