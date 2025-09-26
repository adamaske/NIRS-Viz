headvol = load_vox('C:\nirs\AtlasViewer\Data\Colin\anatomical\headvol.vox', []);


mesh = isosurface(headvol.img,.9);

mesh.vertices = [mesh.vertices(:,2) mesh.vertices(:,1) mesh.vertices(:,3)]; 

% Assumes the 'mesh' structure is already available from the previous steps
% mesh = isosurface(headvol.img, 0.9);
% mesh.vertices = [mesh.vertices(:,2) mesh.vertices(:,1) mesh.vertices(:,3)];

% Define the output OBJ file name
output_file = 'headvol.obj';

% Open the output OBJ file for writing
fid = fopen(output_file, 'w');

% Check if the file was opened successfully
if fid == -1
    error('Could not open file for writing.');
end

% Write a header comment
fprintf(fid, '# OBJ file generated from MATLAB\n');
fprintf(fid, '# Vertices: %d\n', size(mesh.vertices, 1));
fprintf(fid, '# Faces: %d\n', size(mesh.faces, 1));

% Write the vertices
for i = 1:size(mesh.vertices, 1)
    fprintf(fid, 'v %f %f %f\n', mesh.vertices(i, 1), mesh.vertices(i, 2), mesh.vertices(i, 3));
end

% Write the faces
% OBJ face indices are 1-based, which matches MATLAB's indexing
for i = 1:size(mesh.faces, 1)
    fprintf(fid, 'f %d %d %d\n', mesh.faces(i, 1), mesh.faces(i, 2), mesh.faces(i, 3));
end

% Close the file
fclose(fid);

disp(['OBJ file saved as: ', output_file]);