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
x_medio = 0;
y_medio = 0;
z_medio = 0;
j=0;

while (j<50)
    write(stim, [0 1 3 1 0 1 0], "uint8");
    suc = read(stim,3, "uint8");
    teds = read(stim,suc(3), "uint8");


    write(stim, [0 2 3 1 0 1 0], "uint8");
    suc = read(stim,3, "uint8");
    teds = read(stim,suc(3), "uint8");


    write(stim, [0 3 3 1 0 1 0], "uint8");
    suc = read(stim,3, "uint8");
    teds = read(stim,suc(3), "uint8");
    j=j+1;
    end
j=50;

while (j<100)
    write(stim, [0 1 3 1 0 1 0], "uint8");
    suc = read(stim,3, "uint8");
    teds = read(stim,suc(3), "uint8");
    x_medio = x_medio + teds;

    write(stim, [0 2 3 1 0 1 0], "uint8");
    suc = read(stim,3, "uint8");
    teds = read(stim,suc(3), "uint8");
    y_medio = y_medio + teds;

    write(stim, [0 3 3 1 0 1 0], "uint8");
    suc = read(stim,3, "uint8");
    teds = read(stim,suc(3), "uint8");
    z_medio = z_medio + teds;
    j = j+1;
end


x_medio = x_medio/50;
y_medio = y_medio/50;
z_medio = z_medio/50;

% Example of data:
% X:O
% Y:P
% Z:O
while true
    write(stim, [0 1 3 1 0 1 0], "uint8");
    suc = read(stim,3, "uint8");
    teds = read(stim,suc(3), "uint8");
    x = teds - x_medio
    x = round(x/20);

    write(stim, [0 2 3 1 0 1 0], "uint8");
    suc = read(stim,3, "uint8");
    teds = read(stim,suc(3), "uint8");
    y = teds - y_medio;
    y = round(y/20);

    write(stim, [0 3 3 1 0 1 0], "uint8");
    suc = read(stim,3, "uint8");
    teds = read(stim,suc(3), "uint8");
    z = teds - z_medio;
    z = round(z/20);



    x = -1*x*pi/6.4; 
    y = -1*y*pi/6.4;
    z = -1*z*pi/6.4;
   
    X = [X, x];
    
  
    Y = [Y, y];
    
   
    Z = [Z, z];

    % Plot the data
    %ylim([100 150])1
    plot(X, "r");
    plot(Y, "g");
    plot(Z, "b");
    drawnow; write(stim, [0 3 3 1 0 1 0], "uint8");
    suc = read(stim,3, "uint8");
    teds = read(stim,suc(3), "uint8");
end