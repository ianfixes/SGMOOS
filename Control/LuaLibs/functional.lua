 -- Functional Library
 --
 -- @file    functional.lua
 -- @author  Shimomura Ikkei, edits by Ian Katz
 -- @date    2005/05/18
 --
 -- @brief    porting several convenience functional utilities form Haskell,Python etc..


-- mapi(function, table)
-- e.g: mapi(double, {1,2,3})    -> {2,4,6}
function mapi(func, tbl)
   local newtbl = {}
   for i,v in ipairs(tbl) do
      newtbl[i] = func(v)
   end
   return newtbl
end

-- map(function, table)
-- e.g: map(double, {x=1,y=2,z=3})    -> {x=2,y=4,z=6}
function map(func, tbl)
   local newtbl = {}
   for k,v in pairs(tbl) do
      newtbl[k] = func(v)
   end
   return newtbl
end

-- filter(function, table)
-- e.g: filter(is_even, {1,2,3,4}) -> {2,4}
function filter(func, tbl)
   local newtbl = {}
   for i,v in pairs(tbl) do
      if func(v) then
         newtbl[i] = v
      end
   end
   return newtbl
end

-- head(table)
-- e.g: head({1,2,3}) -> 1
function head(tbl)
   return tbl[1]
end

-- tail(table)
-- e.g: tail({1,2,3}) -> {2,3}
 --
 -- XXX This is a BAD and ugly implementation.
-- should return the address to next porinter, like in C (arr+1)
function tail(tbl)
   if table.getn(tbl) < 1 then
         return nil
   end
   
   local newtbl = {}
   local tblsize = table.getn(tbl)
   local i = 2
   while (i <= tblsize) do
      table.insert(newtbl, i - 1, tbl[i])
      i = i + 1
   end
   return newtbl
end

-- foldr(function, default_value, table)
-- e.g: foldr(operator.mul, 1, {1,2,3,4,5}) -> 120
function foldr(func, val, tbl)
   for i,v in pairs(tbl) do
      val = func(val, v)
   end
   return val
end

-- foldri(function, default_value, table)
-- e.g: foldri(operator.mul, 1, {1,2,3,4,5}) -> 120
function foldri(func, val, tbl)
   for i,v in ipairs(tbl) do
      val = func(val, v)
   end
   return val
end

-- reduce(function, table)
-- e.g: reduce(operator.add, {1,2,3,4}) -> 10
function reduce(func, tbl)
   return foldr(func, head(tbl), tail(tbl))
end

-- compose(f,g)
-- e.g: printf = compose(io.write, string.format)
--          -> function(...) return io.write(string.format(unpack(arg))) end
function compose(f,g)
   return function (...)
             return f(g(unpack(arg)))
          end
end


-- bind1(func, binding_value_for_1st)
-- bind2(func, binding_value_for_2nd)
 -- @brief
--      Binding argument(s) and generate new function.
 -- @see also STL's functional, Boost's Lambda, Combine, Bind.
 -- @examples
--      local mul5 = bind1(operator.mul, 5) -- mul5(10) is 5 * 10
--      local sub2 = bind2(operator.sub, 2) -- sub2(5) is 5 -2
function bind1(func, val1)
   return function (val2)
             return func(val1, val2)
          end
end
function bind2(func, val2) -- bind second argument.
   return function (val1)
             return func(val1, val2)
          end
end


-- bindmany(func, arg1, [arg2, ..]) : reduce the number of arguments 
--         required by a function by pre-binding an arbitrary number of them
--
-- e.g.: you have a function: dosomething(x, y, z, a, b, c)
--       bound_dosomething = bindmany(dosomething, x1, y1)
--       bound_dosomething(z1, a1, b1, c1)
function bindmany(f, onearg, ...)
   if not onearg then
      return f
   end
   return bindmany(function(...) return f(onearg, ...) end, ...)
end


-- is(checker_function, expected_value)
 -- @brief
 --      check function generator. return the function to return boolean,
--      if the condition was expected then true, else false.
 -- @example
--      local is_table = is(type, "table")
--      local is_even = is(bind2(math.mod, 2), 1)
--      local is_odd = is(bind2(math.mod, 2), 0)
function is(check, expected)
   return function (...)
             if (check(...) == expected) then
                return true
             else
                return false
             end
          end
end

 -- operator table.
 -- @see also python's operator module.
operator = {
   mod = math.mod;
   pow = math.pow;
   add = function(n,m) return n + m end;
   sub = function(n,m) return n - m end;
   mul = function(n,m) return n * m end;
   div = function(n,m) return n / m end;
   gt  = function(n,m) return n > m end;
   lt  = function(n,m) return n < m end;
   eq  = function(n,m) return n == m end;
   le  = function(n,m) return n <= m end;
   ge  = function(n,m) return n >= m end;
   ne  = function(n,m) return n ~= m end;
   
 }

 -- enumFromTo(from, to)
 -- e.g: enumFromTo(1, 10) -> {1,2,3,4,5,6,7,8,9}
 -- TODO How to lazy evaluate in Lua? (thinking with coroutine)
function enumFromTo(from,to)
   local newtbl = {}
   local step = bind2(operator[(from < to) and "add" or "sub"], 1)
   local val = from
   while val <= to do
      table.insert(newtbl, table.getn(newtbl) + 1, val)
      val = step(val)
   end
   return newtbl
end

 -- make function to take variant arguments, replace of a table.
 -- this does not mean expand the arguments of function took,
 -- it expand the function's spec: function(tbl) -> function(...)
 function expand_args(func)
    return function(...) return func(arg) end
 end



-- reverse(...) : take some tuple and return a tuple of elements in reverse order
--
-- e.g. "reverse(1,2,3)" returns 3,2,1
function reverse(...)
   
   --reverse args by building a function to do it, similar to the unpack() example
   local function reverse_h(acc, v, ...)
      if 0 == select('#', ...) then 
         return v, acc()
      else
         return reverse_h(function () return v, acc() end, ...)
      end
   end
   
   -- initial acc is the end of the list
   return reverse_h(function () return end, ...)
end
         


-- curry(func, num_args) : take a function requiring a tuple for num_args arguments 
--              and turn it into a series of 1-argument functions
-- e.g.: you have a function dosomething(a, b, c)
--       curried_dosomething = curry(dosomething, 3)
--       curried_dosomething (a1) (b1) (c1)
function curry(func, num_args)

   -- currying 2-argument functions seems to be the most popular implementation
   num_args = num_args or 2

   -- helper
   local function curry_h(argtrace, n)
      if 0 == n then
         -- reverse argument list and call function
         return func(reverse(argtrace()))
      else
         -- "push" argument (by building a wrapper function) and decrement n
         return function (onearg)
                   return curry_h(function () return onearg, argtrace() end, n - 1)
                end
      end
   end

   -- no sense currying for 1 arg or less
   if num_args > 1 then 
      return curry_h(function () return end, num_args) 
   else
      return func
   end
end


-- identity function... returns what you pass it
function identity(...)
   return ...
end


-- do any of the items in the table satisfy the criteria function?
function any(criteria_fn, atable)
   
   local function any_h(...)
      if 0 == select("#", ...) then return false end
      if criteria_fn((select(1, ...))) then return true end
      return any_h(select(2, ...))
   end

   return 0 == #atable and nil or any_h(unpack(atable))
end

-- do all of the items in the table satisfy the criteria function?
function all(criteria_fn, atable)
   
   local function all_h(acc, ...)
      if 0 == select("#", ...) then return true end
      if not criteria_fn((select(1, ...))) then return false end
      return all_h(select(2, ...))
   end

   return 0 == #atable and nil or all_h(unpack(atable))
end

