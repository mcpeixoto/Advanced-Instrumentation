function pessoal2
    %% Set up teds and metateds
    %{
    Saber tudo das teds e metateds que é preciso para o programa
    %}
    clear; clc;
    stim = serialport("/dev/ttyACM0", 9600);

    %% AUX FUNCTION
    function ask_values(channel)
        write(stim, [0 channel 3 1 0 1 0], "uint8");
        suc = read(stim,3, "uint8");
    
        if (suc(1) == 1 && suc(2) == 0 && suc(3) == 1)
            return read(stim, 1, "uint8");
        else
            Display("Error reading channel "+ channel +". Error code: " + suc);
            return 0
        end
    end

    
    %%%%%%%%%% Initialize %%%%%%%%%%
    
    fig = figure;
    
    fig.UserData.firstPerson = false; %do we start in 1st person view, or not?
    resetPositionOnBounds = true; %keeps the plane in the region with the matlab logo ground.
    
    hold on
    fig.Position = [600 600 1500 1200];
    
    % Disable axis viewing, don't allow axes to clip the plane, turn on perspective mode.
    fig.Children.Visible = 'off';
    fig.Children.Clipping = 'off';
    fig.Children.Projection = 'perspective';
    
    %% Add the sky - Fancy environment folder ex
    skymap = [linspace(1,0.4,100)',linspace(1,0.4,100)',linspace(1,0.99,100)'];
    
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
    
    %add some extra flat ground going off to (basically) infinity.
    flatground = surf(linspace(-500000,500000,size(ground,1)),linspace(-500000,500000,size(ground,2)),-10001*ones(size(ground)));
    flatground.FaceColor = 'texturemap';
    flatground.CData = texture;
    flatground.AlphaData = 0.8;
    
    camlight('headlight');
    
    camva(40); %view angle
    
    
    
    forwardVec = [1 0 0]';      %Vector of the plane's forward direction in plane frame
    rot = eye(3,3);             %Initial plane rotation
    pos = [-8000,8000,-2000];   %Initial plane position
    vel = 500;                  %Velocity
    % Initial direction of the plane
    x = 0;
    y = 0;
    z = 0;
    
    
    hold off
    axis([-10000 10000 -10000 10000 -10000 10000])
    
    
    %% Animation loop:
    tic
    told = 0;
    
    
    while(ishandle(fig))
        % now is the actual time
        time_stamp_current =  now
        tnew = toc;
    
        %% Ask for values on each axis
        
        % X
        x_ = ask_values(1);
        if x_ ~= 0
            x = x_;
        end
        
        % Y
        y_ = ask_values(2);
        if y_ ~= 0
            y = y_;
        end
    
        % Z
        z_ = ask_values(3);
        if z_ ~= 0
            z = z_;
        end

        % Since angle2dcm uses yaw, pitch, roll
        % I'm assuming this is equivalent to Euler Angles
        % TRY TO USE angle2rod insted of angle2dcm IF I'M WRONG! <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<

        % Normalize vector
        norm = sqrt(x^2 + y^2 + z^2);
        x = x/norm;
        y = y/norm;
        z = z/norm;

        % Também podes testar isto, copilot deu-me assim do nada xD <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<
        % Convert to angles
        yaw = atan2(y, x);
        pitch = atan2(z, sqrt(x^2 + y^2));
        roll = 0;
        % Convert to rotation matrix
        rot = angle2dcm(yaw, pitch, roll);
    
        %Update plane's center position.
        pos = vel*(rot*forwardVec*(tnew-told))' + pos;
        
        %If the plane wants to go under the ground, then bring it back up to the
        %ground surface.
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
        elapsed = elapsed * (10*10^9)
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
            nearestGroundZ = interp2(groundSurf.XData,groundSurf.YData,groundSurf.ZData,cam(1),cam(2));
            if cam(3)<nearestGroundZ
                campos([cam(1),cam(2),nearestGroundZ]);
            end
        
            told = tnew;
            pause(0.0000001);
        
    end
end

