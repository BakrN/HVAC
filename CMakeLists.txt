cmake_minimum_required(VERSION 3.0)
project(Strmgr_test)
add_subdirectory(include)
option(STR "Build strmgr_test" ON)
option(DATA"Build datamgr_test" ON)

set(C_STANDARD 99)
set (SOURCES tests/strgmgr_test.c sensor_db.c )
set (HEADERS sensor_db.h config.h)



add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS})
target_link_libraries(${PROJECT_NAME}  sqlite3)
target_include_directories(${PROJECT_NAME} PUBLIC ${PROJECT_SOURCE_DIR} ${PROJECT_SOURCE_DIR}/tests ${PROJECT_SOURCE_DIR}/include)
