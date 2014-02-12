function stack = imreadstack(filefilter, varargin)
%
% stack = imreadstack(filefilter, varargin)
%
% description:
%     reads a stack of gray scale images into 3d matrix
%
% input:
%     filefilter use images in dir(filefilter) if char array, if cell using specified images
%     varargin   inputs to imread
%
% output:
%     stack      the 3d matrix
%
% See also: imread

if iscell(filefilter)
   filenames = filefilter;
   nz = length(filenames);
else
   d = dir(filefilter);
   filenames = {d(~[d.isdir]).name};
   nz = length(filenames);
   path = fileparts(filefilter);
   for i=1:nz
      filenames{i} = fullfile(path, filenames{i});
   end
end

% load images and check dimesnions
siz = [];
for z=1:nz
   img = imread(filenames{z}, varargin{:});
   if z > 1
      if ~isequal(size(img),siz)
         error('imreadstack: image %s size is not consitent with stack', filenames{z})
      end
      
      stack(:,:,z) = img;     %#ok<AGROW>
   else
      siz = size(img);
      stack = zeros(siz(1), siz(2), nz);
      stack(:,:,1) = img;
   end
end

end
   