function pessoal
close all; clear; clc;

testing = 1; % 1 for testing, 0 for real

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%% INITIAL SETUP %%%%%%%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

if testing == 0
    stim = serialport("/dev/ttyACM0", 9600);

    % First values of x,y and z are discarted. PIC is starting up
    % call get_avg_data for the first 50
    [x_medio, y_medio, z_medio] = get_avg_data(stim, 50);


    % Get the average of the first 100 values of x,y and z
    [x_medio, y_medio, z_medio] = get_avg_data(stim, 100);
end


%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%% SIMULATOR CONFIGURATION FIGURE %%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

fig = figure;

% Configure figure
fig.UserData.firstPerson = false;   % do we start in 1st person view, or not?
resetPositionOnBounds = true;       % keeps the plane in the region with the matlab logo ground.
hold on                             % hold on so we can add multiple objects to the figure
fig.Position = [600 600 1500 1200]; % make the figure a bit bigger

% Disable axis viewing, don't allow axes to clip the plane, turn on perspective mode.
fig.Children.Visible = 'off';
fig.Children.Clipping = 'off';
fig.Children.Projection = 'perspective';

% Add button that says hi
uicontrol('Style', 'pushbutton', 'String', 'Say Hi', 'Position', [20 20 100 20], 'Callback', @SayHi);

% Add the sky
skymap = [linspace(1,0.4,100)', linspace(1,0.4,100)', linspace(1,0.99,100)'];

[skyX,skyY,skyZ] = sphere(50);
sky = surf(500000*skyX,500000*skyY,500000*skyZ,'LineStyle','none','FaceColor','interp');
colormap(skymap);

%% Add the plane - Fancy planes folder
fv = stlread('test.stl');
p1 = patch(fv,'FaceColor',       'red', ...
         'EdgeColor',       'none',        ...
         'FaceLighting',    'gouraud',     ...
         'AmbientStrength', 0.15);
vert = p1.Vertices;

material('metal')

%% Add the ground + textures - Fancy environments folder Ex
texture = imread('texture.jpg');

ground = 10000*membrane(1,40)-10000;
groundSurf = surf(linspace(-10000,10000,size(ground,1)),linspace(-10000,10000,size(ground,2)),ground,'LineStyle','none','AmbientStrength',0.3,'DiffuseStrength',0.8,'SpecularStrength',0,'SpecularExponent',10,'SpecularColorReflectance',1);
groundSurf.FaceColor = 'texturemap';
groundSurf.CData = texture;

% Add some extra flat ground going off to (basically) infinity.
flatground = surf(linspace(-500000,500000,size(ground,1)),linspace(-500000,500000,size(ground,2)),-10001*ones(size(ground)));
flatground.FaceColor = 'texturemap';
flatground.CData = texture;
flatground.AlphaData = 0.8;

camlight('headlight');  % Add a camera light, and tone down the specular highlighting
camva(40);              % View angle

if testing == 1
    % Set keyboard callbacks and flags for movement.
    set(fig,'WindowKeyPressFcn', @KeyPress,'WindowKeyReleaseFcn', @KeyRelease);
    fig.UserData.e = false;
    fig.UserData.q = false;
    fig.UserData.a = false;
    fig.UserData.d = false;
    fig.UserData.w = false;
    fig.UserData.s = false;
end

%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%%%%%%%%% INITIAL AIRPLANE CONFIGURATION %%%%%%%%%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

forwardVec = [1 0 0]';  % Vector of the plane's forward direction in plane frame
rot = eye(3,3);         % Initial plane rotation
pos = [-8000,8000,-2000];   % Initial plane position
vel = 500;              % Velocity


hold off    % Release the hold so we can move the plane around
axis([-10000 10000 -10000 10000 -10000 10000])  % Set the axis limits

%% Animation loop:
tic % start the timer
told = 0; 

while(ishandle(fig))
    % now is the actual time
    time_stamp_current =  now;
    tnew = toc;
    
    %{
        Pedir valores; retirar valores para x,y e z e v (pot)
        xx = x- (x_medio)
        yy = y - (y_medio)
        zz = z - (z_medio)
        Normalizar os valores
        xx = xx/(xx^2 + yy^2 + zz^2)^(1/2)
        yy = yy/(xx^2 + yy^2 + zz^2)^(1/2)
        zz = zz/(xx^2 + yy^2 + zz^2)^(1/2)
        rot = rot*angle2scm(xx, yy, zz)

        # Velocidades é preciso testa as gamas de valores!!!
        v = v*100 (? fator por testar)
    %}

    if testing == 0
        % Read data from PIC
        [x, y, z] = get_data(stim, 1);

        % Remove reference, centering it
        x = x - x_medio;
        y = y - y_medio;
        z = z - z_medio;

        % Round the numbers
        x = fix(x/20);
        y = fix(y/20);
        z = fix(z/20);

        % The maximum value is 6.5 so we're normalizing it 
        x=x*pi/6.5;
        y=y*pi/6.5;
        z=-1*z*pi/6.5;

        rot = rot*angle2dcm(z, y, x);
        pause(0.1);
    else 
        % Check for user inputs:
        if fig.UserData.e
            rot = rot*angle2dcm(0.05,0,0);
        end
        if fig.UserData.q
            rot = rot*angle2dcm(-0.05,0,0);
        end
        if fig.UserData.s
            rot = rot*angle2dcm(0,-0.05,0);
        end
        if fig.UserData.w
            rot = rot*angle2dcm(0,0.05,0);
        end
        if fig.UserData.a
            rot = rot*angle2dcm(0,0,-0.05);
        end
        if fig.UserData.d
            rot = rot*angle2dcm(0,0,0.05);
        end
    end

    % Update plane's center position.
    pos = vel*(rot*forwardVec*(tnew-told))' + pos;
    
    % If the plane wants to go under the ground, then bring it back up to the  ground surface.
    nearestGroundZ = interp2(groundSurf.XData,groundSurf.YData,groundSurf.ZData,pos(1),pos(2));
    if pos(3)<nearestGroundZ
        pos(3) = nearestGroundZ;
    end
    
    if resetPositionOnBounds
        % If we leave the ground area in the X direction, then snap the plane
        % back to the other side.
        if pos(1)>max(groundSurf.XData)
            pos(1) = min(groundSurf.XData);
        elseif pos(1)<min(groundSurf.XData)
            pos(1) = max(groundSurf.XData);
        end

        % If we leave the ground area in the y direction, then snap the plane
        % back to the other side.
        if pos(2)>max(groundSurf.YData)
            pos(2) = min(groundSurf.YData);
        elseif pos(2)<min(groundSurf.YData)
            pos(2) = max(groundSurf.YData);
        end
    end
    
    %Update the plane's vertices using the new position and rotation
    p1.Vertices = (rot*vert')' + repmat(pos,[size(vert,1),1]);

    % Display fps counter on the figure name (window tab) using time_stamp_current
    elapsed = now - time_stamp_current;
    elapsed = elapsed * (10*10^9);
    fig.Name = sprintf('FPS: %0.2f | Hz: %0.2f',1/elapsed,1/elapsed);
    
    %Camera updates:
    if fig.UserData.firstPerson %First person view -- follow the plane from slightly behind.
        camupvec = rot*[0 0 1]';
        camup(camupvec);
        campos(pos' - 1000*rot*[1 0 -0.25]');
        camtarget(pos' + 100*rot*[1 0 0]');    
    else %Follow the plane from a fixed angle
        campos(pos + [-1000,1000,1000/3]);%3000*abs(pos-campos)/norm(pos-campos));
        camtarget(pos);

    end
    
    cam = campos;
    %Also keep the camera from going into the ground (could be done a
    %smarter way to also not look through the ground).
    nearestGroundZ = interp2(groundSurf.XDatsrc,~)
    % Show "TESTING" in the figure title bar.
    src.Name = 'TESTING';
    % Get the current position of the plane.
    pos = src.UserData.p1.Vertices(1,:);a,groundSurf.YData,groundSurf.ZData,cam(1),cam(2));
    if cam(3)<nearestGroundZ
        campos([cam(1),cam(2),nearestGroundZ]);
    end

    told = tnew;
    pause(0.0000001);
    
end

end


function KeyPress(varargin)
    fig = varargin{1};
    key = varargin{2}.Key;
    if strcmp(key,'e') 
        fig.UserData.e = true;
    elseif strcmp(key,'q')
        fig.UserData.q = true;
    elseif strcmp(key,'a')
        fig.UserData.a = true;
    elseif strcmp(key,'d')
        fig.UserData.d = true;
    elseif strcmp(key,'w')
        fig.UserData.w = true;
    elseif strcmp(key,'s')
        fig.UserData.s = true;
    elseif strcmp(key,'v')
        fig.UserData.firstPerson = ~fig.UserData.firstPerson;
    end
end

function KeyRelease(varargin)
    fig = varargin{1};
    key = varargin{2}.Key;
    if strcmp(key,'e') 
        fig.UserData.e = false;
    elseif strcmp(key,'q')
        fig.UserData.q = false;
    elseif strcmp(key,'a')
        fig.UserData.a = false;
    elseif strcmp(key,'d')
        fig.UserData.d = false;
    elseif strcmp(key,'w')
        fig.UserData.w = false;
    elseif strcmp(key,'s')
        fig.UserData.s = false;
    end
end

function [x_medio, y_medio, z_medio] = get_avg_data(stim, n)
    x_medio = 0;
    y_medio = 0;
    z_medio = 0;
    j = 0;
    while (j<n)
        write(stim, [0 1 3 1 0 1 0], "uint8");
        suc = read(stim,3, "uint8");
        teds = read(stim,suc(3), "uint8");
        x_medio = x_medio +teds;
        
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
end

function SayHi(src, ~)
    disp("Hi");
end