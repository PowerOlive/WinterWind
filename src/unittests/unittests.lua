function test_ratp_client()
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

function test_http_client()
	local http = core.create_httpclient()
	local res = http:get("http://google.fr")
	if res.code == 200 then
		return true
	end
	return false
end

function run_unittests()
	-- Init
	local test_results = 0
	local max_tests = 0

	-- test run
	max_tests = max_tests + 1
	if test_ratp_client() then
		test_results = test_results + 1
	end

	max_tests = max_tests + 1
	if test_http_client() then
		test_results = test_results + 1
	end

	-- End Tests
	print(test_results .. "/" .. max_tests .. " LUA tests passed.")
	if test_results ~= max_tests then
		return false
	end
	return true
end

