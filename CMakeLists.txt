cmake_minimum_required(VERSION 3.5)
project(bot)

set(CMAKE_CXX_STANDARD 14)

set(SOURCE_FILES main.cpp)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib -lPocoNet -lPocoFoundation -lPocoUtil -lPocoXML -lPocoCrypto -lPocoNetSSL -ljsoncpp -lPocoData -lPocoDataSQLite")
add_executable(bot ${SOURCE_FILES})

