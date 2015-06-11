

require("functional")

-- this has to agree with PathAction.h
SURGE = 0
SWAY  = 1
HEAVE = 2
ROLL  = 3
PITCH = 4
YAW   = 5

DOF_MIN = SURGE
DOF_MAX = YAW

-- name lookup
DOF_NAME = {"SWAY", "HEAVE", "ROLL", "PITCH", "YAW"}
DOF_NAME[0] = "SURGE"

-- take an unsorted list of moos messages: mail[i][field]
-- return a table of mail[varname][first|last|all|now]
-- where "field" is the field of a moos message (value, name, community, etc)
--  "first" and "last" are the oldest and youngest message.  
--  "all" is a list of all.
--  "now" is the latest value
-- 
-- in other words, this function turns a "for i, v in mail.pairs()" search into
--  myval = mail.MY_VAR and mail.MY_VAR.now or nil
--
function SimplifyMail(vars)

   -- work function to set up basic 3 dimensional array: vars[name][order][field]
   local function packup(acc, v)
      acc[v.name] = acc[v.name] or {all={}}
      acc[v.name].all[#acc[v.name].all + 1] = v
      return acc
   end

   -- add some syntactic sugar
   local function sweeten(msglist)
      msglist.first = msglist.all[1]
      msglist.last  = msglist.all[#msglist.all]
      msglist.now   = msglist.last.value
      return msglist
   end

   return map(sweeten, foldri(packup, {}, vars))
end


-- keep track of the last time we received an update on each var
--  "varlist" is a table of varname -> time
--  "simplemail" is the output of SimplifyMail
-- TODO: optional arg?
function MarkLastUpdated(varlist, simplemail)
   -- iterate through mail
   -- if index doesnt exist, add
   -- if msg time is greater than existing key time, update (implied)

   local function mark(acc, v)
      acc[v.last.name] = v.last.time
      return acc
   end

   return foldr(mark, varlist, simplemail)
end


-- execute a set of CPS functions until one returns nil (non-function)
-- return a function that takes simplified mail as an arg
function Simultaneously(...)
   local isfunction = is(type, "function")
   local funclist = arg


   -- generate a function that executes all functions in the list
   return function (simplemail)

             -- work function, applied to each func in list
             local function checkReturn(acc, a_simultaneous_func)
                -- if one is nil, we're done
                if nil == acc then return nil end

                -- process this function
                local nextfunc = a_simultaneous_func(simplemail)
                
                -- if it's nil, dump the acc... 
                if not isfunction(nextfunc) then return nil end

                -- otherwise, add to it
                acc[#acc + 1] = nextfunc
                return acc

             end -- checkReturn function
             
             -- apply our work function
             local batch = foldri(checkReturn, {}, funclist)
             
             -- same as batch ? simultaneously : nil
             return batch and Simultaneously(unpack(batch))
          end
end


-- execute a set of CPS functions sequentially until all return nil
-- return a function that takes simplified mail as an arg
function Sequentially(...)
   local isfunction = is(type, "function")
   local funclist = arg

   return function (simplemail)
             if 0 == #funclist then
                return nil
             else
                funclist[1] = funclist[1](simplemail)

                if isfunction(funclist[1]) then
                   return Sequentially(funclist[1], select(2, unpack(funclist)))
                else
                   return Sequentially(select(2, unpack(funclist)))
                end
             end
          end
end


function AngleWrap(rads) -- copied from MOOS_ANGLE_WRAP
   rads = rads + math.pi                                -- shift up by pi
   rads = math.mod(rads, 2 * math.pi)                   -- mod
   rads = (rads >= 0) and rads or (rads + 2 * math.pi)  -- put into positive range
   return (0 == rads) and math.pi or (rads - math.pi)   -- shift down, but favor positive pi
end
   

function Pythag(...)
   local sum = 0
   
   for i = 1, select("#", ...) do
      sum = sum + math.pow((select(i, ...)), 2)
   end

   return math.sqrt(sum)
end