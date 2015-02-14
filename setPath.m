function setPath()
%
% path = setPath()
%
% description:
%     sets all necessary paths
%
% See also: initialize

includepaths = {
     '/Base/Classes', ...
     '/Base/ImageFormat',...
     '/Base/Parameter',... 
     '/Base/Tags',...
     '/Base/Algorithms',...
     '/Base/Algorithms/Clustering',...    
     '/Base/Algorithms/Geometry',...     
     '/Base/Algorithms/Geometry/Polygons',...   
     '/Base/Algorithms/GraphTheory',...     
     '/Base/Algorithms/SignalProcessing',...  
     '/Base/ParallelProcessing',...
     '/Base/Utils',...
     ...
     '/ImageProcessing/', ...
     '/ImageProcessing/Filtering', ...
     '/ImageProcessing/Photometry', ...
     '/ImageProcessing/Photometry/Vignetting', ...
     '/ImageProcessing/Segmentation', ...
     '/ImageProcessing/Segmentation/Propagation', ...
     '/ImageProcessing/ShapeDetection', ...
     '/ImageProcessing/Stitching', ...
     '/ImageProcessing/Tracking',...
     '/ImageProcessing/Thresholding', ...
     '/ImageProcessing/Tools',...
     ...
     '/Interface',...
     '/Interface/ImageJ',...
     '/Interface/Imaris',...
     '/Interface/IO',... 
     '/Interface/MetaMorph',...
     '/Interface/Movies',...
     '/Interface/Python',...
     '/Interface/Ilastik',...
     '/Interface/Hugin',...
     ...
     '/Test'...
     ...
     '/Base/Utils/External',...
     ... 
     ... // External Packages
     '/Base/Utils/External/BM3D',...
     '/Base/Utils/External/SLICSuperpixels',...
     '/Base/Utils/External/image_pyramid'
};
 
basepath = fileparts(mfilename('fullpath'));

for p = 1:length(includepaths)
   includepaths{p} = fullfile(basepath, includepaths{p});
end

addpath(includepaths{:});
          
% compability to matlab previous versions
v = version('-release');
if length(v) >= 4
   v = v(1:4);
   if strcmp(v, '2012')
      addpath(fullfile(basepath, '/Base/Utils/External/Matlab2012'));
   end
end

      
end

