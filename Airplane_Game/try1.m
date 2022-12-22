   hFig=figure('Position',[200 200 1600 600]);
      movegui(hFig,'center')  
      %Add pushbutton to view data
      ButtonH=uicontrol('Parent',hFig,'Style','pushbutton','String','View Data','Units','normalized','Position',[0.0 0.5 0.4 0.2],'Visible','on');