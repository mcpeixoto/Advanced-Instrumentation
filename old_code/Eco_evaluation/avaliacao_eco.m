% sudo chown mcpeixoto /dev/ttyACM0
% Reset everything
clear; clc;

% Opening serial conection
s = serialport("/dev/ttyACM0", 9600);


enviado = 1
write(s, enviado, "char");
recebido = read(s,1,"uint8")