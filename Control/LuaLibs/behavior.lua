
require("moos")
require("functional")

-- exposes the functions in this file as a module called behavior
module(..., package.seeall)

local DEFAULT_FORCE          = 20
local MINIMUM_LEAD_DISTANCE  = 1

local isfunction = is(type, "function")



local function readcsv (file)
   local fp = assert(io.open (file))
   local csv = {}
   for line in fp:lines() do
      local row = {}

      -- note: doesn\'t work with strings that contain , values
      for value in line:gmatch("[^,]*") do 
         row[#row+1] = value
      end
      csv[#csv+1] = row
   end

   fp:close()
   return csv
end

local function load_compass_error(file)
   local err_table = {}
   local csv = readcsv(file)
   for _, row in ipairs(csv) do
      local k = tostring(row[1])
      local v = tonumber(row[3])
      -- HAPI_Trace(k, v)
      err_table[k] = v
   end

   return err_table
end

      

local compass_error = load_compass_error("/home/auvlab/missions/microstrain_errors.csv")


-- take a combination of lists and functions, output put a list of (unique) subscriptions
function SubscriptionsFor(...)
   local tmp = {}
   
   --iterate over arguments
   for i = 1, select("#", ...) do
      --extract an item, and if it's a function look up its subscriptions
      local item = select(i, ...)
      if isfunction(item) then
         item = Helper_Subs(item)
      end

      for _, v in ipairs(item) do
         tmp[v] = true
      end
   end

   local ret = {}
   for k, _ in pairs(tmp) do
      ret[#ret + 1] = k
   end
   return ret
end
   

function Helper_Subs(behavior)

-- returns VARS AS KEYS
   if Wait == behavior then
      return {}
   elseif ConstantHeading == behavior then
      return SubscriptionsFor(GoForward, MaintainHeading)
   elseif FaceVerticalPipe == behavior then
      return {} -- TODO
   elseif FaceXYPoint == behavior then
      return SubscriptionsFor(MaintainHeading, {"NAV_X", "NAV_Y"})
   elseif FunctionGenerator == behavior then
      return {} -- TODO
   elseif GoForward == behavior then
      return {} 
   elseif GoUp == behavior then
      return {} 
   elseif GotoAltitude == behavior then
      return {"NAV_ALTITUDE"}
   elseif GotoDepth == behavior then
      return {"NAV_DEPTH"}
   elseif GotoWaypoint == behavior then
      return SubscriptionsFor(FaceXYPoint, GoForward)
   elseif MaintainAltitude == behavior then
      return {"NAV_ALTITUDE"}
   elseif MaintainDepth == behavior then
      return {"NAV_DEPTH"}
   elseif MaintainHeading == behavior then
      return {"NAV_YAW"} 
   elseif ManualControl == behavior then
      return {"MANUAL_SURGE", "MANUAL_SWAY", "MANUAL_HEAVE", 
              "MANUAL_ROLL", "MANUAL_PITCH", "MANUAL_YAW"}
   elseif ManualHeading == behavior then
      return SubscriptionsFor(MaintainHeading, {"MANUAL_HEADING", "MANUAL_HEADING_STEP"})
   elseif Trackline == behavior then
      return SubscriptionsFor(GotoWaypoint)
   elseif XYPattern == behavior then
      return SubscriptionsFor(XYPattern)
   elseif Lawnmower == behavior then
      return SubscriptionsFor(XYPattern)
   else
      HAPI_Trace("Requested subscriptions for an unknown entity:", behavior)
      return {}
   end

end


function AllStop(settings, simplemail)
   for i = DOF_MIN, DOF_MAX do
      HAPI_SetOpenLoop(i, 0, 2, "AllStop")
   end

   return nil
end


function Wait(settings, simplemail)

   if not Helpers_RequireSettings(settings, "Wait", {"wait_seconds"}) then
      return nil
   end

   -- init time if we don't have one
   nowish = HAPI_Time()
   settings.runtime = settings.runtime or {}
   settings.runtime.starttime = settings.runtime.starttime or nowish

   if settings.wait_seconds < nowish - settings.runtime.starttime then return nil end

   return bind1(Wait, settings)
end


function ConstantHeading(settings, simplemail)
   -- combination of goforward and maintainheading
   settings.priority       = settings.priority or 3
   settings.runtime        = settings.runtime or {}
   settings.runtime.label  = settings.runtime.label or "ConstantHeading"

   if not isfunction(Simultaneously(bind1(GoForward, settings),
                                    bind1(MaintainHeading, settings)) (simplemail)) then
      return nil
   end

   return bind1(ConstantHeading, settings)
end


function FaceVerticalPipe(settings, simplemail)
  -- FaceVerticalPipe.cpp
end


function FaceXYPoint(settings, simplemail)
   settings.priority       = settings.priority or 3
   settings.cache          = settings.cache or {}
   settings.runtime        = settings.runtime or {}
   settings.runtime.lu     = settings.runtime.lu or {}
   settings.runtime.label  = settings.runtime.label or "FaceXYPoint"

   -- what to publish
   settings.runtime.publish_fn = settings.runtime.publish_fn or
      function()
         -- notify waypoint_x, waypoint_y, and waypoint_distance
         HAPI_PostDouble("WAYPOINT_X", settings.x_meters)
         HAPI_PostDouble("WAYPOINT_Y", settings.y_meters)
         HAPI_PostDouble("WAYPOINT_DISTANCE", settings.cache.WAYPOINT_DISTANCE)
      end

   -- we need x and y
   if not Helpers_RequireSettings(settings, 
                                         settings.runtime.label, 
                                         {"x_meters", "y_meters"}) then
      return nil
   end

   -- 5 second starvation timeout for NAV x/y
   if not Helpers_CheckStarvation(settings.runtime.label, 
                                           settings.runtime.lu,
                                           simplemail, 
                                           {NAV_X = 5, NAV_Y = 5}) then
      return nil
   end


   -- x and y arrive together, but we'll assume they don't
   if simplemail.NAV_X then settings.cache.NAV_X = simplemail.NAV_X.now end
   if simplemail.NAV_Y then settings.cache.NAV_Y = simplemail.NAV_Y.now end
   
   -- if there was a change, act on it
   if simplemail.NAV_X or simplemail.NAV_Y then
      local xcorrection = settings.x_meters - settings.cache.NAV_X
      local ycorrection = settings.y_meters - settings.cache.NAV_Y

      settings.cache.WAYPOINT_DISTANCE = Pythag(xcorrection, ycorrection)

      -- calculate new heading 
      settings.heading_degrees = math.deg(math.atan2(xcorrection, ycorrection))
      
      settings.runtime.publish_fn() -- publish stuff

   end

   -- do something if we have data
   if settings.cache.NAV_X and settings.cache.NAV_Y then
      MaintainHeading(settings, simplemail)
   end


   return bind1(FaceXYPoint, settings)
end


function GoForward(settings, simplemail)
   local goforward_func = Factory_Go("Forward", SURGE)
   return goforward_func(settings, simplemail)
end


function GoUp(settings, simplemail)
   local goup_func = Factory_Go("Up", HEAVE)
   return goup_func(settings, simplemail)
end


function GoAround(settings, simplemail)
   local goaround_func = Factory_Go("Around", YAW)
   return goaround_func(settings, simplemail)
end

function GotoAltitude(settings, simplemail)
   local gotoalt_func = Factory_Goto("Altitude", "altitude_meters", "NAV_ALTITUDE", HEAVE, false)
   return gotoalt_func(settings, simplemail)
end


function GotoDepth(settings, simplemail)
   local gotodepth_func = Factory_Goto("Depth", "depth_meters", "NAV_DEPTH", HEAVE, true)
   return gotodepth_func(settings, simplemail)
end


function GotoWaypoint(settings, simplemail)
   -- combination of goforward and faceXYpoint
   settings.priority       = settings.priority or 3
   settings.tolerance      = settings.tolerance or 3
   settings.cache          = settings.cache or {}
   settings.runtime        = settings.runtime or {}
   settings.runtime.label  = settings.runtime.label or "GotoWaypoint"
   
   if not isfunction(Simultaneously(bind1(GoForward, settings),
                                    bind1(FaceXYPoint, settings)) (simplemail)) then
      return nil -- we don't expect this, because both behaviors run indefinitely
   end

   -- exit case: cached value from faceXYpoint behavior.
   if settings.cache.WAYPOINT_DISTANCE and
      settings.cache.WAYPOINT_DISTANCE < settings.tolerance then
      
      return nil
   end

   return bind1(GotoWaypoint, settings)

end


function MaintainAltitude(settings, simplemail)
   local maintainalt_func = Factory_Maintain("Altitude", "altitude_meters", 
                                          "NAV_ALTITUDE", "DESIRED_ALTITUDE", HEAVE)
   return maintainalt_func(settings, simplemail)
end


function MaintainDepth(settings, simplemail)
   local function depthcorrection_fn(desired, actual, tolerance)
      local correction_tmp = actual - desired -- depth is negative heave, so correction is negated
      local correction = tolerance < math.abs(correction_tmp) and correction_tmp or 0
      return desired, correction
   end

   local maintaindepth_func = Factory_Maintain("Depth", "depth_meters", 
                                          "NAV_DEPTH", "DESIRED_DEPTH", HEAVE, depthcorrection_fn)
   return maintaindepth_func(settings, simplemail)
end


function MaintainHeading(settings, simplemail)
   local function compensate_compass(yaw_radians)
      --apply correction from table
      local yaw_deg = math.floor(0.5 + math.deg(0 - yaw_radians))
      if yaw_deg > 180 then  yaw_deg = yaw_deg - 360 end
      if yaw_deg < -179 then yaw_deg = yaw_deg + 360 end
      local deg_error = compass_error[tostring(yaw_deg)]
      -- ordinarily we'd subtract, but yaw is negative of degrees

      return yaw_radians + math.rad(deg_error)
   end

   -- PID runs on yaw radians, not heading degrees. we adjust.
   local function yawcorrection_fn(desired, actual, tolerance)
      actual = compensate_compass(actual)

      -- convert desired_heading to desired_yaw
      local desired_yaw = AngleWrap(-1.0 * math.rad(desired))
      local correction_tmp = AngleWrap(desired_yaw - actual)

      local correction = math.rad(tolerance) < math.abs(correction_tmp) and correction_tmp or 0

      -- un-wrap desired location to prevent going the long way around
      local new_desired_yaw = desired_yaw
      if desired_yaw - actual > math.pi then
         new_desired_yaw = desired_yaw - (2 * math.pi)
      elseif desired_yaw - actual < (0 - math.pi) then
         new_desired_yaw = desired_yaw + (2 * math.pi)
      end
      desired_yaw = new_desired_yaw

      return desired_yaw, correction
   end
   
   local maintainheading_func = Factory_Maintain("Heading", "heading_degrees",
                                          "NAV_YAW", "DESIRED_HEADING", YAW, yawcorrection_fn)


   local ret = maintainheading_func(settings, simplemail)
   return ret
end


--MaintainPipeDistance.cpp     
--MaintainPipeRelHeading.cpp   

function ManualControl(settings, simplemail)
   settings.priority       = settings.priority or 3
   settings.runtime        = settings.runtime or {}
   settings.runtime.lu     = settings.runtime.lu or {}
   settings.runtime.label  = settings.runtime.label or "ManualControl"
   settings.cache          = settings.cache or {}

   -- manual surge, sway, heave, roll, pitch, yaw
   -- manual_heading, manual_heading_step, manual_depth, manual_altitude

   local watched_vars = {MANUAL_SURGE = 60,
                         MANUAL_SWAY  = 60,
                         MANUAL_HEAVE = 60,
                         MANUAL_ROLL  = 60,
                         MANUAL_PITCH = 60,
                         MANUAL_YAW   = 60}

   local now = HAPI_Time()

   settings.runtime.lu = Helpers_MarkLastUpdate(settings.runtime.lu, 
                                                watched_vars, 
                                                simplemail, 
                                                now)

   -- for each type of DOF, check if we have an update
   -- if we do, cache it.  if cache is fresh, apply it. otheriwse, stop it. 
   for i = DOF_MIN, DOF_MAX do
      local manvar = "MANUAL_" .. DOF_NAME[i]

      if simplemail[manvar] then
         settings.cache[manvar] = simplemail[manvar].now
      end

      if settings.cache[manvar] and 2 > now - settings.runtime.lu[manvar] then
         HAPI_SetOpenLoop(i, settings.cache[manvar], settings.priority, settings.runtime.label)
      else
         HAPI_SetOpenLoop(i, 0, settings.priority, settings.runtime.label)
      end
   end

   return bind1(ManualControl, settings)

end

function ManualHeading(settings, simplemail)
   settings.priority       = settings.priority or 3
   settings.runtime        = settings.runtime or {}
   settings.runtime.label  = settings.runtime.label or "ManualHeading"

   -- pick up heading or heading step
   if simplemail.MANUAL_HEADING then
      settings.heading_degrees = simplemail.MANUAL_HEADING.now
   elseif simplemail.MANUAL_HEADING_STEP then
      if settings.heading_degrees then
         local newheading =  settings.heading_degrees + simplemail.MANUAL_HEADING_STEP.now
         settings.heading_degrees = ((180 + newheading) % 360) - 180
      end
   end

   -- don't start until we specify a heading
   if settings.heading_degrees then
      if not isfunction(MaintainHeading(settings, simplemail)) then 
         return nil 
      end
   end 

   return bind1(ManualHeading, settings)
end


function Trackline(settings, simplemail)

   settings.priority       = settings.priority or 3
   settings.cache          = settings.cache or {}
   settings.runtime        = settings.runtime or {}
   settings.runtime.label  = settings.runtime.label or "Trackline"
   settings.lead_meters    = settings.lead_meters or MINIMUM_LEAD_DISTANCE
   settings.henrik_ratio   = settings.henrik_ratio or 0.5

   settings.runtime.publish_fn = identity -- don't let FaceXYPoint publish waypoints

   -- stuff we can't provide defaults for
   if not Helpers_RequireSettings(settings, settings.runtime.label,
                                           {"do_sliding_waypoint", 
                                            "do_sway_to_track",
                                            "dest_x_meters",
                                            "dest_y_meters"}) then
      return nil
   end

   -- init origin from current location if we haven't yet
   if not (settings.orig_x_meters and settings.orig_y_meters) then

      -- we need to deal with NAV_X/_Y locally... allow 5 seconds for them to show
      if not Helpers_CheckStarvation(settings.runtime.label, 
                                              settings.runtime.lu,
                                              simplemail, 
                                              {NAV_X = 5, NAV_Y = 5}) then
         return nil
      end
      
      -- x and y arrive together, but we'll assume they don't
      if simplemail.NAV_X then settings.cache.NAV_X = simplemail.NAV_X.now end
      if simplemail.NAV_Y then settings.cache.NAV_Y = simplemail.NAV_Y.now end

      -- we now know origin and trackline angle
      if not (settings.cache.NAV_X and settings.cache.NAV_Y) then
         return bind1(Trackline, settings) -- better luck next time
      else
         settings.orig_x_meters = settings.cache.NAV_X
         settings.orig_y_meters = settings.cache.NAV_Y
         settings.runtime.theta_trackline = 
            math.atan2(settings.dest_y_meters - settings.orig_y_meters,
                       settings.dest_x_meters - settings.orig_x_meters)
      end
   end
   
   -- figure out where we are in relation to the trackline
   local x                = settings.cache.NAV_X
   local y                = settings.cache.NAV_Y
   local x_correction     = settings.dest.x_meters - x
   local y_correction     = settings.dest_y_meters - y
   local goal_distance    = Pythag(x_correction, y_correction)
   local theta_vehicle    = atan2(x_correction, y_correction)
   local theta_trackline  = settings.runtime.theta_trackline
   local henrik           = settings.henrik_ratio
   
   -- the line between x_error, y_error and current location crosses the
   -- trackline at the point that is goal_distance from the goal 
   local x_error    = goal_distance * math.cos(theta_vehicle - theta_trackline)
   
   -- we can use x_error to find the point on the trackline that is closest to the 
   -- vehicle's position -- on the line that is normal to the trackline
   local x_normal   = settings.dest.x_meters - x_error * math.cos(theta_trackline)
   local y_normal   = settings.dest.y_meters - x_error * math.sin(theta_trackline)
   local track_err  = Pythag(x - x_normal, y - y_normal)

   if settings.lead_meters < goal_distance then
      return nil -- SUCCESS
   end
   
   -- notify trackline x and trackline y (goal), and trackline_error()
   -- maybe also trackline_distance
   HAPI_PostDouble("TRACKLINE_X",        settings.dest_x_meters)
   HAPI_PostDouble("TRACKLINE_Y",        settings.dest_y_meters)
   HAPI_PostDouble("WAYPOINT_X",         settings.dest_x_meters)
   HAPI_PostDouble("WAYPOINT_Y",         settings.dest_y_meters)
   HAPI_PostDouble("WAYPOINT_DISTANCE",  goal_distance)
   HAPI_PostDouble("TRACKLINE_ERROR",    track_err)

   -- determine where next waypoint goes
   if settings.do_sliding_waypoint then
     
      local newmode = ""

      -- use the henrik ratio to gracefully move toward the trackline
      if x_error * henrik > settings.lead_meters then
         newmode = "TRANSIT" -- go directly towards the trackline
         settings.x_meters = x_normal + settings.lead_meters * math.cos(theta_trackline)
         settings.y_meters = y_normal + settings.lead_meters * math.sin(theta_trackline)
      else
         newmode = "APPROACH" -- go to a spot henrik_ratio between the normal and goal
         settings.x_meters = (1 - henrik) * x_normal + henrik * settings.dest_x_meters
         settings.y_meters = (1 - henrik) * y_normal + henrik * settings.dest_y_meters
      end

      -- done setting mode, report it
      if newmode ~= settings.runtime.trackline_mode then
         HAPI_Trace("trackline switching mode to " .. newmode)
         settings.runtime.trackline_mode = newmode
      end
   end

   -- run waypoint
   if not isfunction(GotoWaypoint(settings, simplemail)) then
      return nil
   end

   -- sway to the line if that will work in our favor
   if settings.do_sway_to_track then
      if 90 > math.abs(theta_vehicle - theta_trackline) then
         local sign = (theta_trackline > theta_vehicle) and -1 or 1
         HAPI_SetClosedLoop(SWAY, track_err * sign, 0.0, 
                            settings.priority, settings.runtime.label)
      end
   end

end


function Lawnmower(settings, simplemail)

   settings.priority       = settings.priority or 3
   settings.runtime        = settings.runtime or {}
   settings.runtime.label  = settings.runtime.label or "XYPattern"

   -- stuff we can't provide defaults for
   if not Helpers_RequireSettings(settings, settings.runtime.label,
                                           {"do_sliding_waypoint", 
                                            "do_sway_to_track",
                                            "tracklength_meters",
                                            "tracks",
                                            "spacing_meters",
                                            "angle_degrees",
                                            "x_meters",
                                            "y_meters"}) then
      return nil
   end

   
   function displace(off_h, off_v)
      local p = {}
      local a = math.rad(settings.angle_degrees)
      local ctr_x = settings.x_meters
      local ctr_y = settings.y_meters
      p.x_meters = ctr_x + ( off_v * math.sin(a) + off_h * math.cos(a))
      p.y_meters = ctr_y + (-off_h * math.sin(a) + off_v * math.cos(a))
      return p
   end

   settings.waypoints = {}
   for n = 0, (tracks - 1) do
      local p1 = displace(n * spacing_meters, 0)
      local p2 = displace(n * spacing_meters, tracklength_meters)

      if 0 == n % 2 then -- go up
         settings.waypoints[#settings.waypoints] = p1
         settings.waypoints[#settings.waypoints] = p2
      else
         settings.waypoints[#settings.waypoints] = p2
         settings.waypoints[#settings.waypoints] = p1
      end
   end

   return XYPattern(settings, simplemail)
end


function XYPattern(settings, simplemail)

   settings.priority       = settings.priority or 3
   settings.runtime        = settings.runtime or {}
   settings.runtime.label  = settings.runtime.label or "XYPattern"

   -- stuff we can't provide defaults for
   if not Helpers_RequireSettings(settings, settings.runtime.label,
                                           {"do_sliding_waypoint", 
                                            "do_sway_to_track",
                                            "waypoints"}) then
      return nil
   end

   local tlines = {}
   local orig_x = nil
   local orig_y = nil

   -- iterate through points and make a new settings pack
   -- we aren't recursively copying the settings, so there will be shared references
   -- but that's ok because we will run sequentially and this is a closure
   for _, p in settings.waypoints do
      newsettings = {}
      for k, v in pairs(settings) do
         newsettings[k] = v
      end
      
      if not (orig_x and orig_y) then
         newsettings.x_meters = p.x_meters
         newsettings.y_meters = p.y_meters
         tlines[#tlines] = bind1(GotoWaypoint, newsettings)
      else
         newsettings.orig_x_meters = orig_x
         newsettings.orig_y_meters = orig_y
         newsettings.dest_x_meters = p.x_meters
         newsettings.dest_y_meters = p.y_meters
         tlines[#tlines] = bind1(Trackline, newsettings)
      end
      orig_x = p.x_meters
      orig_y = p.y_meters

   end

   -- now that we've made the sequence, execute the first bit
   return Sequentially(unpack(tlines)) (simplemail)

end




-- apply an open loop force indefinitely
--
-- label  is the suffix of the behavior name, as in "Forward"
-- dof    is the axis on which the force will be applied, as in SURGE
function Factory_Go(label, dof)
   
   local function ret(settings, simplemail)
      settings.priority       = settings.priority or 3
      settings.force_percent  = settings.force_percent or DEFAULT_FORCE
      settings.runtime        = settings.runtime or {}
      settings.runtime.label  = settings.runtime.label or "Go" .. label
      
      HAPI_SetOpenLoop(dof, settings.force_percent, settings.priority, settings.runtime.label)
      
      return bind1(ret, settings)
   end
   
   return ret
end





-- apply a closed loop force indefinitely
--
-- label          is the suffix of the behavior name, as in MaintainDepth
-- settingname    is the key into the settings table for the value of the target, as in depth
-- moosvar        is the variable that the target value will be compared to, as in NAV_DEPTH
-- desiredvar     is the moos variable that we will publish, as in DESIRED_DEPTH
-- dof            is the method that will be used to apply the force, as in HEAVE
-- correction_fn  calculates correction, based on desired, actual, and tolerance
function Factory_Maintain(label, settingname, moosvar, desiredvar, dof, correction_fn)

   -- default behavior is the correction with tolerance applied
   correction_fn = correction_fn or function(desired, actual, tolerance)
                                       local corr_tmp = desired - actual
                                       local correction = math.abs(corr_tmp) > tolerance and corr_tmp or 0
                                       return desired, correction
                                    end

   -- correction = desired - actual
   local function action_fn(settings, simplemail)

      local desired = settings[settingname]
      local correction = nil

      desired, correction = correction_fn(desired, simplemail[moosvar].now, settings.tolerance)

      HAPI_PostDouble(desiredvar, desired)
      HAPI_SetClosedLoop(dof, correction, desired, settings.priority, settings.runtime.label)

      return true
   end

   local force_fn = Factory_LoopForce("Maintain", identity, action_fn) 

   return force_fn(label, settingname, moosvar, dof)
end


-- apply a positive or negative open loop force until a goal is reached.
--
-- label        is the suffix of the behavior name, as in GotoDepth
-- settingname  is the key into the settings table for the value of the target, as in depth
-- moosvar      is the variable that the target value will be compared to, as in NAV_DEPTH
-- dof          is the method that will be used to apply the force, as in HEAVE
-- isnegative   controls whether the correction and resulting force have the same sign
--                  as in, positive depth is negative heave
function Factory_Goto(label, settingname, moosvar, dof, isnegative)

   -- open loop needs a force
   local function settings_fn(settings)
      settings.force_percent = settings.force_percent or DEFAULT_FORCE
      return settings
   end

   -- get the proper sign for the force and apply it
   --
   -- settings    is a table of the settings
   -- simplemail  is a table of simplified mail
   local function action_fn(settings, simplemail)
      local desired     = settings[settingname]
      local actual      = simplemail[moosvar].now
      local correction  = desired - actual
      local sign        = (correction > 0 and isnegative) and -1.0 or 1.0
      local force       = settings.force_percent * sign

      HAPI_Trace("desired =", desired, 
                 " :: actual =", actual, 
                 " :: force =", force)

      if settings.tolerance > math.abs(correction) then 
         return false 
      else
         HAPI_Trace("NOT EXITING",
                    " :: corr = ", correction,
                    " :: tol =", settings.tolerance)
      end

      HAPI_SetOpenLoop(dof, force, settings.priority, settings.runtime.label)

      return true
   end

   local force_fn = Factory_LoopForce("Goto", settings_fn, action_fn)
   return force_fn(label, settingname, moosvar, dof)
end

-- general function for classes of single DOF open/closed loop force application
-- 
-- baselabel    is the prefix for the name of the class of behaviors, as in "Maintain"
-- settings_fn  handles any setting initialization 
-- action_fn    applies the force and returns false when it's time to exit
function Factory_LoopForce(baselabel, settings_fn, action_fn)


   -- function for a class of functions
   -- 
   -- label          is the suffix for the behavior, as in "Depth"
   -- settingname    is the key to the settings array where the target value is held
   -- moosvar        is the name of the variable contianing the actual (current) value
   -- dof            is what axis will get the force, as in SURGE
   return function (label, settingname, moosvar, dof)
             
             local behaviorname = baselabel .. label
             

             -- the actual behavior function
             local function ret(settings, simplemail)
                settings.priority       = settings.priority or 3
                settings.cache          = settings.cache or {}
                settings.runtime        = settings.runtime or {}
                settings.runtime.lu     = settings.runtime.lu or {}
                settings.runtime.label  = settings.runtime.label or behaviorname
                settings.tolerance      = settings.tolerance or 0

                -- custom settings init
                settings = settings_fn(settings)
                
                -- make sure we have a proper settings file
                if not Helpers_RequireSettings(settings, behaviorname, {settingname}) then
                   return nil
                end
                
                -- 5 second starvation timeout
                local starvation_tbl = {}
                starvation_tbl[moosvar] = 5
                if not Helpers_CheckStarvation(behaviorname, 
                                                        settings.runtime.lu,
                                                        simplemail, 
                                                        starvation_tbl) then
                   return nil
                end
                
                if simplemail[moosvar] then -- cache it
                   settings.cache[moosvar] = simplemail[moosvar]
                elseif settings.cache and settings.cache[moosvar] then -- use cache
                   simplemail[moosvar] = settings.cache[moosvar]
                end
 
                -- don't hit the action_fn until we get our first mail value
                if simplemail[moosvar] then
                   if not action_fn(settings, simplemail) then -- we are done
                      return nil
                   end
                end
                
                return bind1(ret, settings) -- pass continuation
             end
             
             return ret
          end
end


-- settings is the table of settings
-- behavior name goes in the error message
-- setting_namelist is all the settings that need checking
function Helpers_RequireSettings(settings, behaviorname, setting_namelist)
   local function criteria_fn (settingname)
      if nil ~= settings[settingname] then return true end
      HAPI_MissionMessage(settingname .. " not supplied in settings for " .. behaviorname)
      return false
   end

   return all(criteria_fn, setting_namelist)
end

-- lastupdate_table is a variable that holds the lastupdated structure
-- simplemail is the incoming mail
-- moosvar_table is keyed with the varname and valued with the max delay on receiving an update
function Helpers_CheckStarvation(behaviorname, lastupdate_table, simplemail, moosvar_table)

   local thetime = HAPI_Time()
   
   lastupdate_table = Helpers_MarkLastUpdate(lastupdate_table, 
                                             moosvar_table, 
                                             simplemail, 
                                             thetime)
                
   -- input starvation is a fail case
   local msg = "Starvation! " .. behaviorname .. " didn't get an update of: "
   local fail = false
   for moosvar, timeout in pairs(moosvar_table) do
      if timeout < thetime - lastupdate_table[moosvar] then
         fail = true
         msg = msg .. moosvar .. " within " .. timeout .. " seconds, "
      end
   end

   if fail then
      HAPI_MissionMessage(msg)
      return false
   end

   return true
end


--mark a bunch of values with their last_update time.  
function Helpers_MarkLastUpdate(lastupdate_table, moosvar_table, simplemail, thetime)
   if nil == lastupdate_table then
      HAPI_Trace("Bad call to Helpers_MarkLastUpdate: nil lastupdate_table")
   end

   -- make sure all the moosvars we want are in the lastupdate_table
   -- if this is a var's first time through, init with the current time
   for k, _ in pairs(moosvar_table) do
      lastupdate_table[k] = lastupdate_table[k] or thetime
   end

   -- record updates for all vars in lastupdate_table
   return MarkLastUpdated(lastupdate_table, simplemail)
end