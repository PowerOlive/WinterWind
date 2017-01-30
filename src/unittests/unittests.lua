
function run_unittests()
	local schedules=core.get_ratp_schedules(1)
	if schedules == nil then
		return false
	end
	for i = 1, #schedules do
		if schedules[i].destination == nil or schedules[i].hour == nil then
			return false
		end
	end

	return true
end

