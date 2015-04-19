macro(add_target_to_test targetName testName testFile)
    set(current_target ${targetName}.${testName})
    add_executable(${current_target} ${testFile})
    target_link_libraries(${current_target} gtest gtest_main ${ARGN})
    add_test(NAME ${current_target}
             COMMAND ${current_target})
endmacro(add_target_to_test)


add_target_to_test(Core Storage TestStorage.cpp core glog)