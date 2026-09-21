# Unit tests.  Each test is one executable registered with add_test.
# Tests that need assets/AE000.DAT or AE001.DAT must skip (exit code 77 is
# treated as SKIP) when the files are absent.
set(EMPIRES_ASSET_DIR "${CMAKE_SOURCE_DIR}/assets" CACHE PATH "Directory holding the original AE000.DAT/AE001.DAT")
set(EMPIRES_FIXTURE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/fixtures")

function(empires_add_test name)
  add_executable(${name} ${ARGN})
  target_link_libraries(${name} PRIVATE empires_core)
  target_compile_definitions(${name} PRIVATE
    EMPIRES_ASSET_DIR="${EMPIRES_ASSET_DIR}"
    EMPIRES_FIXTURE_DIR="${EMPIRES_FIXTURE_DIR}")
  add_test(NAME ${name} COMMAND ${name})
  set_tests_properties(${name} PROPERTIES SKIP_RETURN_CODE 77)
endfunction()
