
-- exposes the functions in this file as a module called behavior
module(..., package.seeall)

function new()
   return {}
end

function add(aset, avalue)
   aset[avalue] = true
   return aset
end

function remove(aset, avalue)
   aset[avalue] = nil
   return aset
end

function clear(aset)
   return {}
end

function merge(...)
   local ret = {}
   for i = 1, select("#", ...) do
      for k, v in pairs(select(i, ...)) do
         ret[k] = v
      end
   end

   return ret
end


function tolist(aset)
   local ret = {}
   for k, _ in pairs(aset) do
      ret[#ret] = k
   end
   
   return ret
end

