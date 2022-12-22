% Create a 3D plot of the environment
[x,y] = meshgrid(-10:0.1:10,-10:0.1:10);
z = x .* exp(-x.^2 - y.^2);
figure
mesh(x,y,z)
axis([-10 10 -10 10 -1 1])
xlabel('X')
ylabel('Y')
zlabel('Z')

% Create the airplane model
vertices = [0 0 0; 1 0 0; 1 1 0; 0 1 0; 0 0 1; 1 0 1; 1 1 1; 0 1 1];
faces = [1 2 3 4; 2 6 7 3; 4 3 7 8; 1 5 8 4; 1 2 6 5; 5 6 7 8];
patch('Vertices',vertices,'Faces',faces,...
      'FaceColor','blue','EdgeColor','none')

% Implement WASD controls
while true
    k = get(gcf,'currentkey');
    switch k
        case 'w'
            % Move the airplane forward
        case 'a'
            % Move the airplane left
        case 's'
            % Move the airplane backward
        case 'd'
            % Move the airplane right
        otherwise
            % Do nothing
    end
end
