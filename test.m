
% t - type - META (1) or TEDS (0)
% c - channel
% r - read/write
% o - offset
% v - value

clc; clear;
stim = serialport("/dev/ttyACM0", 9600);


Tvw = [0 4 3 2 0 2 0 1];
suc = [];
teds = [];


write(stim,Tvw, "uint8");
suc = read(stim,3, "uint8");
display(suc);