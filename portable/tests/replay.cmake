# Deterministic integration replays (headless SDL dummy drivers).  Each
# manifest under portable/tests/replay/ scripts a run and pins the frame
# hashes; they need the game executable and the original assets (skip = 77).
if(EMPIRES_BUILD_APP)
  find_package(Python3 COMPONENTS Interpreter QUIET)
  if(Python3_Interpreter_FOUND)
    file(GLOB EMPIRES_REPLAYS CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/replay/*.json")
    foreach(manifest ${EMPIRES_REPLAYS})
      get_filename_component(name ${manifest} NAME_WE)
      add_test(NAME replay_${name}
               COMMAND ${Python3_EXECUTABLE} ${CMAKE_SOURCE_DIR}/tools/portable/replay_test.py
                       $<TARGET_FILE:empires> ${manifest} --assets ${EMPIRES_ASSET_DIR})
      set_tests_properties(replay_${name} PROPERTIES SKIP_RETURN_CODE 77 TIMEOUT 600)
    endforeach()
  endif()
endif()
