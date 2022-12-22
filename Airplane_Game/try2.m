

function [a, b] = myfun(x, y)
    % b=x, a=y
    a = y;
    b = x;
end

% test it
x = 1:10;
y = 10:-1:1;
[a, b] = myfun(x, y);
disp(a)