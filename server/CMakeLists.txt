cmake_minimum_required(VERSION 3.13)
project(angustifolia_server C)

set(CMAKE_C_STANDARD 99)
set(PROJECT_SOURCE_DIR "./src")

#openssl.
add_library(openssl STATIC IMPORTED)
set_target_properties(openssl PROPERTIES
		IMPORTED_LOCATION "/home/lucky-fish/openssl/libcrypto.a"
		INTERFACE_INCLUDE_DIRECTORIES /home/lucky-fish/openssl/include)
#openssl-ssl
add_library(openssl_ssl STATIC IMPORTED)
set_target_properties(openssl_ssl PROPERTIES
		IMPORTED_LOCATION "/home/lucky-fish/openssl/libssl.a")
#dl
add_library(dl STATIC IMPORTED)
set_target_properties(dl PROPERTIES
		IMPORTED_LOCATION "/usr/lib/libdl.so")

#mysql
add_library(mysql STATIC IMPORTED)
set_target_properties(mysql PROPERTIES
		IMPORTED_LOCATION "/usr/lib/libmysqlclient.so"
		INTERFACE_INCLUDE_DIRECTORIES /usr/include/mysql)

#threads
add_library(pthread STATIC IMPORTED)
set_target_properties(pthread PROPERTIES
		IMPORTED_LOCATION "/usr/lib/libpthread.so")

#math
add_library(math STATIC IMPORTED)
set_target_properties(math PROPERTIES
		IMPORTED_LOCATION "/usr/lib/libm.so")

#network section
add_library(mcres_license_server_network src/network/network.c src/network/packet.h src/network/packet.c src/network/network.h)

#crypto section
add_library(mcres_license_server_crypto src/crypto/ecc.h src/crypto/ecc.c src/crypto/base64.c src/crypto/base64.h)

#mysql section
add_library(mcres_license_server_mysql src/mysql/mcres_mysql.c src/mysql/mcres_mysql.h)

#motd section
add_library(mcres_license_server_motd src/motd/motd.c src/motd/motd.h)

#life cycle
add_library(mcres_license_server_lifecycle src/lifecycle/connection_handler.h src/lifecycle/connection_handler.c src/lifecycle/event_handling.c src/lifecycle/event_handling.h)

#other tools
add_library(mcres_license_server_util src/util/util.h src/util/time.c src/util/random.c src/util/signal.c src/util/memory.c)

#core implemention
add_executable(mcres_license_server src/main.c)

add_dependencies(mcres_license_server mcres_license_server_crypto)
add_dependencies(mcres_license_server mcres_license_server_network)
add_dependencies(mcres_license_server mcres_license_server_mysql)
add_dependencies(mcres_license_server mcres_license_server_lifecycle)
add_dependencies(mcres_license_server mcres_license_server_util)
add_dependencies(mcres_license_server mcres_license_server_motd)

target_link_libraries(mcres_license_server_network mcres_license_server_crypto)
target_link_libraries(mcres_license_server mcres_license_server_network)
target_link_libraries(mcres_license_server mcres_license_server_lifecycle)
target_link_libraries(mcres_license_server_lifecycle mcres_license_server_network)
target_link_libraries(mcres_license_server_lifecycle mcres_license_server_motd)
target_link_libraries(mcres_license_server_network mcres_license_server_util)
#target_link_libraries(mcres_license_server_motd mcres_license_server_util)
target_link_libraries(mcres_license_server mcres_license_server_mysql)
target_link_libraries(mcres_license_server_lifecycle mcres_license_server_mysql)

target_link_libraries(mcres_license_server_crypto openssl)
target_link_libraries(mcres_license_server_crypto openssl_ssl)
target_link_libraries(mcres_license_server_mysql mysql)
target_link_libraries(mcres_license_server_network pthread)

target_link_libraries(mcres_license_server_crypto dl)

target_link_libraries(mcres_license_server_lifecycle math)
