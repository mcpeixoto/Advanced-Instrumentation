
% t - type - META (1) or TEDS (0)
% c - channel
% r - read/write
% o - offset
% v - value


stim = serialport("/dev/ttyACM0", 9600);


suc = [];
teds = [];

o = 0; % offset 0
c = 1; % channel 1
v = 1; % value 0
r = 0; % read 0


Mt = [0 0 1 2 0 2 1 o]; % For metateds
Tt = [0 c 1 2 0 2 3 o];
Tvr = [0 c 3 1 0 1 o];
Tvw = [0 c 3 2 0 2 o v];


% Ask metateds
write(stim, [0 0 1 2 0 2 1 0], "uint8");
suc = read(stim,3, "uint8");
if (suc(1) ==0)
    display("Erro nas metateds")
    flag=0;
else
    teds = read(stim,suc(3), "uint8");
    display(teds);
    flag=1;
end




% Ask channel 1 info
write(stim, [0 1 1 2 0 2 3 0], "uint8");
suc = read(stim,3, "uint8");
if (suc(1) ==0)
    display("Erro channel 1 info")
    flag=0;
else
    teds = read(stim,suc(3), "uint8");
    display(teds);
    flag=1;
end



% Read channel 1
write(stim, [0 1 3 1 0 1 0], "uint8");
suc = read(stim,3, "uint8");
if (suc(1) ==0)
    display("Erro ao ler sensor 1")
    flag=0;
else
    teds = read(stim,suc(3), "uint8");
    display(teds);
    flag=1;
end