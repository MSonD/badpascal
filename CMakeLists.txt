cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
project(badpas)
add_subdirectory(fsmm)
add_custom_target(make_lexer
  COMMAND fsmm lexer.fs.cpp lexer.cpp
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Generating lexer source"
)
set(SRC "symtable.cpp" "lexer.cpp" "lexer_base.cpp" "data.cpp"  "parse_base.cpp")
add_executable(badpas main.cpp ${SRC})
add_executable(badpas_tests tests.cpp ${SRC})
add_dependencies(badpas make_lexer)
include_directories(badpas PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
install(TARGETS badpas RUNTIME DESTINATION bin)
