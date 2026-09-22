# config.c unit tests (empires.json host configuration store).
empires_add_test(test_config test_config.c)
set_tests_properties(test_config PROPERTIES WORKING_DIRECTORY "${CMAKE_CURRENT_BINARY_DIR}")
