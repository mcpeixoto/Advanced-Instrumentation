clear; clc;

% Abrir porta 
stim = serialport("/dev/ttyACM0", 9600);

% Pedir metated
display("A Metated é :");
write(stim, [0 0 1 2 0 2 1 0], "uint8");
suc = read(stim,3, "uint8");
if (suc(1) ==0)
    display("Erro na Metated")
    suc
    flag=0;
else
    teds = read(stim,suc(3), "uint8");
    display(teds);
    flag=1;
end

% Pedir Teds
i=1;
display("Ler teds")
while (i<8)
    display("Canal");
    display(i);
    write(stim, [0 i 1 2 0 2 3 0], "uint8");
    suc = read(stim,3, "uint8");
if (suc(1) ==0)
    display("Erro ao ler Ted canal");
    display(i);
    flag=0;
else
    teds = read(stim,suc(3), "uint8");
    display(teds);
    flag=1;
end
i = i+1;
end

% Ler canais
i = 1;
display("Ler os canais associados ao sensor");
while (i<4)
    display("Canal");
    display(i);
    write(stim, [0 i 3 1 0 1 0], "uint8");
suc = read(stim,3, "uint8")
if (suc(1) ==0)
    display("Erro ao ler sensor");
    flag=0;
else
    value = read(stim,suc(3), "uint8");
    display(value);
    flag=1;
end
i = i+1;
end

% Vamos escrever nos leds
display("Escrever nos leds 1 a 1");
i=4;
while (i<8)
    write(stim, [0 i 3 2 0 2 0 1], "uint8");
    suc = read(stim,3, "uint8");
    pause(0.2)
    i=i+1;
end
i=4;
while (i<8)
    write(stim, [0 i 3 2 0 2 0 0], "uint8");
    suc = read(stim,3, "uint8");
    pause(0.2)
   
    i= i+1;
end

