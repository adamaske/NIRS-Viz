[v, f] = read_surf('C:\nirs\AtlasViewer\Data\Colin\anatomical\pialsurf.mesh');

% 2. Define Output File Name
output_file = 'pialsurf.obj';

% 3. Open File for Writing
% 'w' mode opens the file for writing (creates it if it doesn't exist, overwrites if it does)
fid = fopen(output_file, 'w');

if fid == -1
    error('Could not open file for writing: %s', output_file);
end

% 4. Write Header Comments (Optional, but good practice)
fprintf(fid, '# OBJ file exported from MATLAB\n');
fprintf(fid, '# Vertices: %d\n', size(v, 1));
fprintf(fid, '# Faces: %d\n', size(f, 1));

% 5. Write Vertices ('v')
% Each row in 'v' becomes a 'v x y z' line in the OBJ file
for i = 1:size(v, 1)
    % %f is used for floating-point numbers (vertex coordinates)
    fprintf(fid, 'v %f %f %f\n', v(i, 1), v(i, 2), v(i, 3));
end

% 6. Write Faces ('f')
% Each row in 'f' becomes an 'f i1 i2 i3' line, where i1, i2, i3 are vertex indices.
% MATLAB (1-indexed) faces match the OBJ (1-indexed) format, so no adjustment is needed.
for i = 1:size(f, 1)
    % %d is used for integers (vertex indices)
    fprintf(fid, 'f %d %d %d\n', f(i, 1), f(i, 2), f(i, 3));
end

% 7. Close the File
fclose(fid);

disp(['Successfully saved mesh to: ', output_file]);