% Reset everything
clear; clc;
% sudo chown mcpeixoto /dev/ttyACM0

% List available ports
% serialportlist("available")

% Opening serial conection
s = serialport("/dev/ttyACM0", 9600);

data = read(s,1,"uint8");


% Closing serial conection
% delete(s)


% data is an array of characters
data = char(data)

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
    data = read(s,1,"string");

    % If data is: :
    % : , . or new line, ignore it
    % If data is X, Y or Z then read the next batch of data
    % and store it in the corresponding array
    if data == ":" | data == "," | data == "." | data == "\n" | data == '' | data == ' '
        continue;
    elseif data == "X"
        % Read the ":"
        read(s,1,"string");
        % Read the next batch of data
        X = [X, read(s,1,"uint8")];
    elseif data == "Y"
        % Read the ":"
        read(s,1,"string");
        % Read the next batch of data
        Y = [Y, read(s,1,"uint8")];
    elseif data == "Z"
        % Read the ":"
        read(s,1,"string");
        % Read the next batch of data
        Z = [Z, read(s,1,"uint8")];
    else 
        % Comunicate error
        disp("Error: Unknown data");
        % Show data
        disp(data);
    end


    
    % Plot the data
    %ylim([100 150])
    plot(X, "r");
    plot(Y, "g");
    plot(Z, "b");
    drawnow;
end