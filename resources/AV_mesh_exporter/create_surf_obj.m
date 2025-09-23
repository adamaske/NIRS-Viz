% Define the input file path
surface_path = "C:\nirs\AtlasViewer\Data\Colin\anatomical\pialsurf.mesh";


% Define the output OBJ file name
surf_output_file = 'brain_lowpoly_.obj';

% Read the mesh data using the custom function
% v - vertices, f - faces
[v, f] = read_surf(surface_path);

% The faces array 'f' is typically 1-indexed in MATLAB, 
% while OBJ files are also 1-indexed. No adjustment is needed here.

% Open the output OBJ file for writing
fid = fopen(surf_output_file, 'w');

% Check if the file was opened successfully
if fid == -1
    error('Could not open file for writing.');
end

% Write a header comment
fprintf(fid, '# OBJ file generated from MATLAB\n');
fprintf(fid, '# Vertices: %d\n', size(v, 1));
fprintf(fid, '# Faces: %d\n', size(f, 1));

% Write the vertices
for i = 1:size(v, 1)
    fprintf(fid, 'v %f %f %f\n', v(i, 1), v(i, 2), v(i, 3));
end

% Write the faces
for i = 1:size(f, 1)
    fprintf(fid, 'f %d %d %d\n', f(i, 1), f(i, 2), f(i, 3));
end

% Close the file
fclose(fid);

disp(['OBJ file saved as: ', surf_output_file]);