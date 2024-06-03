cmake_minimum_required(VERSION 3.0.0)
project(mpcto2pc VERSION 0.1.0)


find_package(cpprestsdk REQUIRED)
find_package(Boost REQUIRED program_options)
find_package(Catch2 CONFIG REQUIRED)
find_package(cryptopp CONFIG REQUIRED)
find_package(Protobuf CONFIG REQUIRED)
find_package(gRPC CONFIG REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(yaml-cpp CONFIG REQUIRED)
# Set the path to GDB
set(CMAKE_GDB usr/bin/gdb)

# Use GDB as the debugger
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g")
include(CTest)
enable_testing()

set(CMAKE_CXX_STANDARD 17)

find_package(Catch2 REQUIRED)
set(TEST_SOURCES
    test/prg.cpp
    test/tiiprime.cpp
    test/rest.cpp
)
set(TEST_DEPS
    src/crypt_util.cpp 
    src/tii_prime.cpp
    src/rest_server.cpp
    src/rest_client.cpp
)
set(TEST_LIBS
    Catch2::Catch2
    Catch2::Catch2WithMain
    cryptopp-static
    cpprest 
    ssl crypto
    pthread
    ${PROTOBUF_LIBRARIES} 
    ${GRPC_LIBRARIES}
)
add_executable(tests ${TEST_SOURCES} ${TEST_DEPS})
target_link_libraries(tests PRIVATE ${TEST_LIBS})

add_executable(tests_mysql test/mysql_test.cpp src/mysql.cpp src/config_loader.cpp)
target_link_libraries(tests_mysql PRIVATE mysqlcppconn yaml-cpp stdc++fs Catch2::Catch2 Catch2::Catch2WithMain)

add_executable(tests_postgresql test/postgresql_test.cpp src/postgresql.cpp src/config_loader.cpp)
target_link_libraries(tests_postgresql PRIVATE pqxx yaml-cpp stdc++fs Catch2::Catch2 Catch2::Catch2WithMain)

# generate keys and certificate for https unit tests
add_custom_target(
    keys ALL
    COMMAND openssl ecparam -genkey -name prime256v1 -noout -out test-private-key.pem
    COMMAND openssl ec -in test-private-key.pem -pubout -out test-public-key.pem
    COMMAND openssl req -new -x509 -sha256 -key test-private-key.pem -subj /CN=localhost -out test-certificate.pem
    COMMENT "Generating keys and certificate for https unit tests"
    BYPRODUCTS test-private-key.pem test-public-key.pem test-certificate.pem
)
# Proto file
get_filename_component(test_proto "protos/test.proto" ABSOLUTE)
get_filename_component(test_proto_path "${test_proto}" PATH)

set(VCPKG_TOOL_PATH "/opt/vcpkg/installed/x64-linux/tools")
# Generated files will be in the corresponding build directory
set(proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/test.pb.cc")
set(proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/test.pb.h")
set(grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/test.grpc.pb.cc")
set(grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/test.grpc.pb.h")

# Generating protobuf files for hello-world testing
add_custom_command(
    OUTPUT "${proto_srcs}" "${proto_hdrs}"
    COMMAND ${VCPKG_TOOL_PATH}/protobuf/protoc
    ARGS --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
         -I "${test_proto_path}"
         "${test_proto}"
    DEPENDS "${test_proto}"
)
# Generating gRPC files
add_custom_command(
    OUTPUT "${grpc_srcs}" "${grpc_hdrs}"
    COMMAND ${VCPKG_TOOL_PATH}/protobuf/protoc
    ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
         --plugin=protoc-gen-grpc="${VCPKG_TOOL_PATH}/grpc/grpc_cpp_plugin"
         -I "${test_proto_path}"
         "${test_proto}"
    DEPENDS "${proto_srcs}"
)

include_directories(${CMAKE_CURRENT_BINARY_DIR})  # Add the directory containing generated files
include_directories(/opt/vcpkg/installed/x64-linux/include/grpc++ 
                    /opt/vcpkg/installed/x64-linux/include/grpcpp 
                    /opt/vcpkg/installed/x64-linux/include/grpc )

add_executable(grpc_test test/grpc.cpp ${proto_hdrs} ${proto_srcs} ${grpc_srcs} ${grpc_hdrs})
target_link_libraries(grpc_test PRIVATE protobuf::libprotobuf gRPC::grpc++ gRPC::grpc++_reflection Catch2::Catch2 Catch2::Catch2WithMain)

# Proto file
get_filename_component(shares_proto "protos/shares.proto" ABSOLUTE)
get_filename_component(shares_proto_path "${shares_proto}" PATH)
# Generated files will be in the corresponding build directory
set(shares_proto_srcs "${CMAKE_CURRENT_BINARY_DIR}/shares.pb.cc")
set(shares_proto_hdrs "${CMAKE_CURRENT_BINARY_DIR}/shares.pb.h")
set(shares_grpc_srcs "${CMAKE_CURRENT_BINARY_DIR}/shares.grpc.pb.cc")
set(shares_grpc_hdrs "${CMAKE_CURRENT_BINARY_DIR}/shares.grpc.pb.h")

# Generating protobuf files for mo->dealer testing
add_custom_command(
    OUTPUT "${shares_proto_srcs}" "${shares_proto_hdrs}"
    COMMAND ${VCPKG_TOOL_PATH}/protobuf/protoc
    ARGS --cpp_out "${CMAKE_CURRENT_BINARY_DIR}"
         -I "${shares_proto_path}"
         "${shares_proto}"
    DEPENDS "${shares_proto}"
)
# Generating gRPC files
add_custom_command(
    OUTPUT "${shares_grpc_srcs}" "${shares_grpc_hdrs}"
    COMMAND ${VCPKG_TOOL_PATH}/protobuf/protoc
    ARGS --grpc_out "${CMAKE_CURRENT_BINARY_DIR}"
         --plugin=protoc-gen-grpc="${VCPKG_TOOL_PATH}/grpc/grpc_cpp_plugin"
         -I "${shares_proto_path}"
         "${shares_proto}"
    DEPENDS "${shares_proto_srcs}"
)

set(DEALER_SRC src/dealer.cpp src/mysql.cpp src/postgresql.cpp src/tii_prime.cpp 
src/crypt_util.cpp src/rest_server.cpp src/db_schemas.cpp src/config_loader.cpp)
add_executable(dealer ${DEALER_SRC} ${shares_proto_hdrs} ${shares_proto_srcs} ${shares_grpc_srcs} ${shares_grpc_hdrs} )
target_link_libraries(dealer protobuf::libprotobuf gRPC::grpc++ gRPC::grpc++_reflection
mysqlcppconn pqxx ${Boost_LIBRARIES} ssl crypto cryptopp-static yaml-cpp stdc++fs gmp pthread cpprest)

set(MODEL_OWNER_SRC src/model_owner.cpp src/mysql.cpp src/postgresql.cpp src/tii_prime.cpp 
src/crypt_util.cpp src/rest_server.cpp src/rest_client.cpp src/db_schemas.cpp src/config_loader.cpp)
add_executable(model_owner ${MODEL_OWNER_SRC} ${shares_proto_hdrs} ${shares_proto_srcs} ${shares_grpc_srcs} ${shares_grpc_hdrs})
target_link_libraries(model_owner mysqlcppconn pqxx ssl crypto
                    protobuf::libprotobuf gRPC::grpc++ gRPC::grpc++_reflection
                    cpprest pthread gmp ${Boost_LIBRARIES}
                    cryptopp-static yaml-cpp stdc++fs)

set(SUPER_DEALER_SRC src/super_dealer.cpp src/mysql.cpp src/postgresql.cpp src/tii_prime.cpp
src/crypt_util.cpp src/rest_server.cpp src/db_schemas.cpp src/config_loader.cpp)
add_executable(super_dealer ${SUPER_DEALER_SRC})
target_link_libraries(super_dealer mysqlcppconn pqxx ssl crypto
                        cryptopp-static cpprest pthread  ${Boost_LIBRARIES}
                        yaml-cpp stdc++fs)

set(CLIENT_SRC src/client.cpp src/mysql.cpp src/postgresql.cpp src/tii_prime.cpp 
src/crypt_util.cpp  src/rest_client.cpp src/db_schemas.cpp src/config_loader.cpp)
add_executable(client ${CLIENT_SRC})
target_link_libraries(client mysqlcppconn pqxx gRPC::grpc++ ssl crypto
                    cpprest pthread  gmp ${Boost_LIBRARIES}
                    cryptopp-static yaml-cpp stdc++fs)

set(CPACK_PROJECT_NAME ${PROJECT_NAME})
set(CPACK_PROJECT_VERSION ${PROJECT_VERSION})
include(CPack)

