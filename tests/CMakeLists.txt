macro(hb_add_test category target)
    add_executable(test_${target} ${category}/${target}.cpp)
    target_link_libraries(test_${target} PRIVATE harbour)
    target_compile_features(test_${target} PRIVATE cxx_std_20)
    add_test(NAME ${category}:${target} COMMAND test_${target})
    set_property(TEST ${category}:${target} PROPERTY LABELS "${category}")
endmacro()

# #############################
# Server Tests
# #############################
hb_add_test(server http)
hb_add_test(server ssl)
#hb_add_test(server routes)

# #############################
# HTTP Tests
# #############################
hb_add_test(http formdata)
hb_add_test(http requests)
hb_add_test(http cookies)

# #############################
# Crypto Tests
# #############################
hb_add_test(crypto aes)
hb_add_test(crypto base64)
hb_add_test(crypto base64url)
hb_add_test(crypto random)
hb_add_test(crypto securecookies)
hb_add_test(crypto jwt)