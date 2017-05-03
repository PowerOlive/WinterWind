function test_strings()
	local score = 4
	local b64enc = core.base64_encode("lua_b64")
	if b64enc ~= "bHVhX2I2NA==" then
		print("test_strings: base64_encode test failed")
		score = score - 1
	end

	if core.base64_decode(b64enc) ~= "lua_b64" then
		print("test_strings: base64_decode test failed")
		score = score - 1
	end

	local json_test = "{\"testkey\":\"value\"}"
	local json_res = core.read_json(json_test)
	if json_res == nil or json_res.testkey ~= "value" then
		print("test_strings: read_json test failed")
		score = score - 1
	end

	local json_write_var={}
	json_write_var["testkey2"] = "value2"
	json_res = core.write_json(json_write_var)
	if json_res ~= '{"testkey2":"value2"}\n' then
		print("test_strings: write_json test failed")
		score = score - 1
	end
	return 4,score
end

function test_ratp_client()
	local schedules=core.get_ratp_schedules("RER_A", "Antony")
	if schedules == nil then
		return 1,0
	end
	for i = 1, #schedules do
		if schedules[i].destination == nil or schedules[i].hour == nil then
			return 1,0
		end
	end

	return 1,1
end

function test_http_client()
	local score = 4
	local http = core.create_httpclient()
	http:add_uri_param("lang", "de")
	local res = http:get("http://www.google.fr")
	if res.code ~= 200 then
		print("test_http_client: get test failed (rc: " .. res.code .. ")")
		score = score - 1
	end

	res = http:delete("http://www.amazon.com/")
	if res.code ~= 200 then
		print("test_http_client: delete test failed (rc: " .. res.code .. ")")
		score = score - 1
	end

	http:add_form_param("search", "lua")
	res = http:post("http://httpbin.org/post")
	if res.code ~= 200 then
		print("test_http_client: post test failed (rc: " .. res.code .. ")")
		score = score - 1
	end

	res = http:get_json("http://q.uote.me/api.php?p=json&l=1&s=random")
	if res.data[1].text == nil then
		print("test_http_client: get_json test failed.")
		score = score - 1
	end

-- Doesn't work, blocked at the request's end
--	res = http:head("http://www.cnn.com")
--	if res.code ~= 200 then
--		print("test_http_client: head test failed (rc: " .. res.code .. ")")
--		return false
--	end
	return 4,score
end

function test_postgresql()
	local max_score = 2
	local score = 2
	local pg = core.create_postgresql_client("host=postgres user=unittests dbname=unittests_db password=un1Ttests")
	if pg == nil then
		return max_score,0
	end

	if not pg:register_statement("test_stmt_lua", "SELECT * FROM pg_stats") then
		score = score - 1
	end

	return max_score, score
end
function run_unittests(run)
	if not run then
		print("Lua tests skipped by C++ part")
		return true
	end
	-- Init
	local test_results = 0
	local max_tests = 0
	local t,s

	-- test run
	t,s = test_strings()
	max_tests = max_tests + t
	test_results = test_results + s

	t,s = test_ratp_client()
	max_tests = max_tests + t
	test_results = test_results + s

	t,s = test_http_client()
	max_tests = max_tests + t
	test_results = test_results + s

	t,s = test_postgresql()
	max_tests = max_tests + t
	test_results = test_results + s

	print()
	-- End Tests
	if test_results ~= max_tests then
		print("Lua NOK (" .. test_results .. "/" .. max_tests .. ")")
		return false
	end

	print("Lua OK (" .. max_tests .. " tests)")
	return true
end

