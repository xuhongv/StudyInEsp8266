-----------------------------------------------------------------------------------
-- Ali provide lua function for data convertion, "device meta <--lua table --> alink json".  --
-- Developer can compose specific script for each desired device by implementing --
-- all of Transaction Functions listed as below.                                 --
--                                                                               --
-- Transaction Functions:                                                        --
--   is_increment():                                                             --
--     @Desc:Design for device which cannot handle sigle-attribute command.      --
--           Lua author need to set "false" as return value and server will      --
--           compose a full-attributes command by integrate other attributes     --
--           recorded on server. Otherwise, set "true".                          --
--     @Para:<null>                                                              --
--     @Rets:true/false                                                          --
--                                                                               --
--   private_cmd_query(json):                                                    --
--     @Desc:Invoked by server when app query device status.                     --
--           Lua author need to compose a "query" cmd for device                 --
--     @Para:<null>                                                              --
--     @Rets:"query" cmd string for device status                                --
--                                                                               --
--   convert_from_private(meta):                                                 --
--     @Desc:Invoked by server, convert device meta data to alink json data.     --
--     @Para:device meta data                                                    --
--     @Rets:alink json data                                                     --
--                                                                               --
--   convert_to_private(json):                                                   --
--     @Desc:Invoked by server, convert alink json data to device meta data.     --
--     @Para:alink json data                                                     --
--     @Rets:device meta data                                                    --
--                                                                               --
-- Basic Functions:                                                              --
--   string.fromhex(str)                                                         --
--   string.tohex(str)                                                           --
--   getbyte(str,pos)                                                            --
--   subbyte(str,start,end)                                                      --
--   arraytostring(tdlist, start, end)                                           --
--   serialize(obj)                                                              --
-----------------------------------------------------------------------------------

---Begin: Transaction Functions---
  function is_increment()
    local ret_sample = false;
    -- do your job here --
    return ret_sample;                   
  end
  
  function private_cmd_query(json)
     local private_cmd_sample = "010101010101"
     -- do your job here --
     return private_cmd_sample
  end
  
  function convert_from_private(meta)
    local jsonDataSample = {
		OnOff_Power= {
	        value= 1
	    },
	    Color_Temperature= {
	        value= 1
	    },
	    Light_Brightness= {
	        value= 1
	    },
	    TimeDelay_PowerOff= {
	        value= 1
	    },
	    WorkMode_MasterLight= {
	        value = 1
	    }
          }
    -- do your job here --
    local str = string.fromhex(meta)
    local v_OnOff_Power = bit32.band(str:byte(3), 0x1);
--	print(tostring(v_OnOff_Power));
	local v_WorkMode_MasterLight = bit32.band(str:byte(4), 0xf);
--	print(tostring(v_WorkMode_MasterLight));
	local v_Color_Temperature = bit32.band(str:byte(5), 0xff);
--	print(tostring(v_Color_Temperature));
	local v_Light_Brightness = bit32.band(str:byte(6), 0xff);
--	print(tostring(v_Light_Brightness));
	local v_TimeDelay_PowerOff = bit32.band(str:byte(7), 0xff);
--	print(v_TimeDelay_PowerOff);

	jsonDataSample={
		OnOff_Power = {value = tostring(v_OnOff_Power)},
		WorkMode_MasterLight = {value = tostring(v_WorkMode_MasterLight)},
		Color_Temperature = {value = tostring(v_Color_Temperature)},
		Light_Brightness = {value = tostring(v_Light_Brightness)},
		TimeDelay_PowerOff = {value = tostring(v_TimeDelay_PowerOff)},
	}

    -- do your job end --
    return jsonDataSample
  end

  function convert_to_private(t)
    local metaDataListSample = {
		[0] = 0xaa,       --header
		[1] = 0x07,       --cmd length
		[2] = 0x01,       --OnOff_Power
		[3] = 0x01,       --WorkMode_MasterLight
		[4] = 0x01,       --Color_Temperature
		[5] = 0x00,       --Light_Brightness time
		[6] = 0x00,       --TimeDelay_PowerOff
		[7] = 0x55,       --end
    }
    local metaDatamaxposSample = 7
    -- do your job here --
	for key, value in pairs(t) do
	--print(key)
		if (key == "OnOff_Power") then
			metaDataListSample[2] = tonumber(t['OnOff_Power']['value'])
			--print(key, metaDataListSample[2])
		elseif (key == "WorkMode_MasterLight") then
			metaDataListSample[3] = tonumber(t['WorkMode_MasterLight']['value'])
			--print(key, metaDataListSample[3])
		elseif (key == "Color_Temperature") then
			metaDataListSample[4] = tonumber(t['Color_Temperature']['value'])
			--print(key, metaDataListSample[4])
		elseif (key == "Light_Brightness") then
			metaDataListSample[5] = tonumber(t['Light_Brightness']['value'])
			--print(key, metaDataListSample[5])
		elseif (key == "TimeDelay_PowerOff") then
			metaDataListSample[6] = tonumber(t['TimeDelay_PowerOff']['value'])
			--print(key, metaDataListSample[6])
		end
	end
    
    -- do your job end --
    local metaData = arraytostring(metaDataListSample,0,metaDatamaxposSample)
    return metaData
  end
---End: Transaction Functions---

---Begin: Basic Functions---
  function string.fromhex(str)
      return (str:gsub('..', function (cc)
          return string.char(tonumber(cc, 16))
      end))
  end
  
  function string.tohex(str)
      return (str:gsub('.', function (c)
          return string.format('%02X', string.byte(c))
      end))
  end
  
  function getbyte(str,pos)
    local b15 = string.sub(str, (pos*2)+1, ((pos+1)*2))
    return b15
  end
  
  function subbyte(str,pos1,pos2)
    local b15 = string.sub(str, ((pos1-1)*2)+1, (pos2*2)+1)  
    return b15
  end
  
  function arraytostring(tdlist, pos1, pos2)
    local res ="";
    for i=pos1,pos2 do
      res = res..string.char(tdlist[i])
    end
    res = string.tohex(res)
    return res;
  end
  
  function serialize(obj)  
    local lua = ""  
    local t = type(obj)  
    if t == "number" then  
      lua = lua .. obj  
    elseif t == "boolean" then  
      lua = lua .. tostring(obj)  
    elseif t == "string" then  
      lua = lua .. string.format("%q", obj)  
    elseif t == "table" then  
      lua = lua .. "{"  
      for k, v in pairs(obj) do  
        lua = lua .. serialize(k) .. ":"..serialize(v) .. ","  
      end  
      local metatable = getmetatable(obj)  
        if metatable ~= nil and type(metatable.__index) == "table" then  
          for k, v in pairs(metatable.__index) do  
            lua = lua .. serialize(k) .. ":"..serialize(v) .. ","  
          end  
        end  
      local luaLen =  string.len(lua)-1;
      lua = string.sub(lua,1,luaLen)
      lua = lua .. "}"  
    elseif t == "nil" then  
      return nil  
    else  
      error("can not serialize a " .. t .. " type.")  
    end  
    return lua  
  end
---End: Basic Functions---

---Begin: lua test---
--[[
  function lua_test()
    local luatableDataSample = {
		OnOff_Power= {
	        value= 1
	    },
	    Color_Temperature= {
	        value= 48
	    },
	    Light_Brightness= {
	        value= 80
	    },
	    TimeDelay_PowerOff= {
	        value= 0
	    },
	    WorkMode_MasterLight= {
	        value = 1
	    }
    }
	local metaDataSample = 'AA07010130500055'
    print(convert_to_private(luatableDataSample))
    print(serialize(convert_from_private(metaDataSample)))
  end
  
  lua_test()
--]]
---END: lua test---

