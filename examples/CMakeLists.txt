macro(hb_add_example target)
    add_executable(example_${target} ${target}.cpp)
    target_link_libraries(example_${target} PRIVATE harbour)
    target_compile_features(example_${target} PRIVATE cxx_std_23)
endmacro()

# #############################
# Simple Examples
# #############################
hb_add_example(handlers)
hb_add_example(templates)
hb_add_example(files)
hb_add_example(routes)
hb_add_example(dock)
hb_add_example(logging)

# #############################
# Intermediate examples
# #############################
hb_add_example(json)
hb_add_example(forms)
hb_add_example(cookies)
hb_add_example(database)
hb_add_example(middleware)

# #############################
# Advanced examples
# #############################
add_subdirectory(blog)
hb_add_example(securecookies)
hb_add_example(websockets)