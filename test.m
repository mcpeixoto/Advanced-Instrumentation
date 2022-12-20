
% t - type - META (1) or TEDS (0)
% c - channel
% r - read/write
% o - offset
% v - value

clc; clear;
stim = serialport("/dev/ttyACM0", 9600);


write(stim, [0 1 3 1 0 1 0], "uint8");
suc = read(stim,3, "uint8")
if (suc(1) ==0)
    display("Erro ao ler sensor 1")
    flag=0;
else
    teds = read(stim,suc(3), "uint8");
    display(teds);
    flag=1;
end


