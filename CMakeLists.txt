cmake_minimum_required(VERSION 3.1)
set(CMAKE_CXX_STANDARD 11)
project(badpas)
add_subdirectory(fsmm)
add_custom_target(make_lexer
  COMMAND fsmm lexer.fs.cpp lexer.cpp
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
  COMMENT "Generating lexer source"
)
add_executable(badpas symtable.cpp main.cpp lexer.cpp lexer_base.cpp data.cpp)
add_dependencies(badpas make_lexer)
target_include_directories(badpas PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
install(TARGETS badpas RUNTIME DESTINATION bin)
