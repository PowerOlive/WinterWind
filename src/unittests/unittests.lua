
function run_unittests()
	if core.get_ratp_schedules(1) == nil then
		return false
	end
	return true
end

