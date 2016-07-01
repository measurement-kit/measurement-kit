function mk_dns_query(dns_class, dns_type, dns_query)
    return {
        operation = "dns_query",
        dns_class = dns_class,
        dns_type = dns_type,
        query = dns_query,
        nameserver = "8.8.8.8:53",
    }
end

local res = mk_yield({
    mk_dns_query("IN", "A", "google.com"),
    mk_dns_query("IN", "A", "www.kernel.org"),
    mk_dns_query("IN", "A", "slashdot.org")
})
for key, value in pairs(res) do
    print(key)
    for subkey, subvalue in pairs(value) do
        print("  ", subkey, ":", subvalue)
    end
end
print(res)
print(res[1])
print(res[2])
print(res[3])
