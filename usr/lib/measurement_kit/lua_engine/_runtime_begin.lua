-- Part of measurement-kit <https://measurement-kit.github.io/>.
-- Measurement-kit is free software. See AUTHORS and LICENSE for more
-- information on the copying conditions.

mk = (function ()
    local self = {}
    self.coroutines = {}

    self.async = function (func)
        self.coroutines[coroutine.create(func)] = true
    end

    self._call_cxx = function (dispatch)
        local coro, _ = coroutine.running()
        if self.coroutines[coro] == nil then
            error("dns_query() called from outside a coroutine")
        end
        self.coroutines[coro] = false     -- make the coroutine non runnable
        dispatch(coro)                    -- dispatch call to C++ API
        result, err = coroutine.yield()   -- wait for C++ API to return
        self.coroutines[coro] = true      -- make runnable again
        coroutine.yield()                 -- wait for main loop to restart us
        return err, result
    end

    self.dns_query = function (dns_class, dns_type, dns_query, settings)
        return self._call_cxx(function (coro)
            self._dns_query(coro, dns_class, dns_type, dns_query, settings)
        end)
    end

    self._loop = function ()
        repeat
            local coroutines = {}
            local count = 0
            for coro, is_not_blocked in pairs(self.coroutines) do
                if is_not_blocked then
                    coroutine.resume(coro)
                end
                if coroutine.status(coro) ~= "dead" then
                    coroutines[coro] = self.coroutines[coro]
                    count = count + 1
                end
            end
            self.coroutines = coroutines
            self._loop_once()
        until count <= 0
    end

    return self
end)()

-- Make sure we find extra packages
package.path = "usr/lib/measurement_kit/lua_engine/?.lua"
