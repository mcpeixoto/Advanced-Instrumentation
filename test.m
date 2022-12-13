
% t - type - META (1) or TEDS (0)
% c - channel
% r - read/write
% o - offset
% v - value

clc; clear;
stim = serialport("/dev/ttyACM0", 9600);


% Ask metateds
write(stim, [0 0 1 2 0 2 1 0], "uint8");

suc = read(stim, 6, "uint8")


