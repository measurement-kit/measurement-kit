-- Part of measurement-kit <https://measurement-kit.github.io/>.
-- Measurement-kit is free software. See AUTHORS and LICENSE for more
-- information on the copying conditions.

local pprint = require("pprint")

function dns_query_all(nameserver, qname)
    local result
    local err
    local settings = {
        ["dns/nameserver"] = nameserver,
        ["dns/attempts"] = 1,
        ["dns/timeout"] = 0.66
    }
    local addresses = {}
    err, result = mk.dns_query("IN", "A", qname, settings)
    pprint(err, result)
    err, result = mk.dns_query("IN", "AAAA", qname, settings)
    pprint(err, result)
end

mk.async(
    function ()
        dns_query_all("8.8.8.8:53", "nexa.polito.it")
    end,
    function ()
        dns_query_all("8.8.8.8:53", "ooni.torproject.org")
    end)
