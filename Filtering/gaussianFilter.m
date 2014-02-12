function out = gaussianFilter(image, varargin)
%
% out = gaussianFilter(image, ksize, radius, sigma)
%
% description:
%    apply a gaussian filter with std sigma and radius to an image
%
% input:
%    image     image to be filtered
%    ksize     h x w (x l) size of filter
%    sigma     standard deviation of the Gaussian Kernel
%    padding   padding of array at borders
%
% output:
%   out        filtered image
%
% See also: fspecial2, fspecial3, imfilter

out = linearFilter(image, 'gaussian', varargin{:});

end