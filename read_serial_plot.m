% Reset everything
clear; clc;
% sudo chown mcpeixoto /dev/ttyACM0

% List available ports
% serialportlist("available")

% Opening serial conection
stim = serialport("/dev/ttyACM0", 9600);



% Closing serial conection
% delete(s)


% data is an array of characters

% Create X, Y, Z dynamic arrays
X = [];
Y = [];
Z = [];



% Initialize a figure to plot the 3 variables on a 2D graph
figure(1);
hold on;
grid on;
xlabel("Time");
ylabel("Acceleration (m/s^2)");
title("Acceleration vs Time");



% Example of data:
% X:O
% Y:P
% Z:O
while true
    write(stim, [0 1 3 1 0 1 0], "uint8");
    suc = read(stim,3, "uint8");
    teds = read(stim,suc(3), "uint8");
    X = [X, teds];
    
    write(stim, [0 2 3 1 0 1 0], "uint8");
    suc = read(stim,3, "uint8");
    teds = read(stim,suc(3), "uint8");
    Y = [Y, teds];
    
    write(stim, [0 3 3 1 0 1 0], "uint8");
    suc = read(stim,3, "uint8");
    teds = read(stim,suc(3), "uint8");
    Z = [Z, teds];

    % Plot the data
    %ylim([100 150])1
    plot(X, "r");
    plot(Y, "g");
    plot(Z, "b");
    drawnow;
end