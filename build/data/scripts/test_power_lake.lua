DEBUG = truelast_healing_times = {}heal_period = 2.0healing_per_second = 10done = truefunction onTouched(touched_object)		--if DEBUG then LogError("Touched " .. GetObjectName(touched_object:GetObject())) end		last_heal_time = last_healing_times[GetObjectName(touched_object:GetObject())]	if last_heal_time == nil or GetTime() - last_heal_time > heal_period then		heal_value = healing_per_second / heal_period				if IsDerivedOrExactClass(touched_object:GetObject(), "Character") then			if DEBUG then LogError("Touched " .. GetObjectName(touched_object:GetObject())) end			Character.AddHealth(touched_object:GetObject(), heal_value)		end				last_healing_times[GetObjectName(touched_object:GetObject())] = GetTime()	endend