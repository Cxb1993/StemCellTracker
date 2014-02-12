function b = isimarisid(id)
%
% b = isimarisid(id)
%
% description:
%    checks if id is ImarisApplication, and integer i or a string that is 
%    convertible to an id


b = 0;

if isa(id, 'Imaris.IApplicationPrxHelper')
   b = 1;
   return
end

if isnumeric(id)
   b = 1;
   return
end

if ~isnan(str2double(id))
   b = 1;
   return
end
      