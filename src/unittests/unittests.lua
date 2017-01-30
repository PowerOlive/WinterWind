
function run_unittests()
	local schedules=core.get_ratp_schedules(1)
	if schedules == nil then
		return false
	end
	print(schedules)
	return true
end

