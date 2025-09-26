
dirnameAtlas = 'C:/nirs/AtlasViewer/';
dirnameSubj = 'C:/nirs/AtlasViewer/Data/Colin/';
searchPaths = {dirnameSubj; dirnameAtlas};

% Debug info

headvol = initHeadvol()

% Load all objects


searchPath = 'C:\nirs\AtlasViewer\Data\Colin\anatomical';

headvol    = getHeadvol(headvol, searchPath);
headsurf   = getHeadsurf(headsurf, searchPath);
pialsurf   = getPialsurf(pialsurf, searchPath);

