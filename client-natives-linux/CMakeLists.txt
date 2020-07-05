cmake_minimum_required(VERSION 3.13)
project(angustifolia_linux_client C ASM)

set(CMAKE_C_STANDARD 99)
#set(BUILD_SHARED_LIBS true)
set(PROJECT_SOURCE_DIR "./src")

add_compile_options("-fPIC")

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
		IMPORTED_LOCATION "/usr/lib/x86_64-linux-gnu/libdl.so")

include_directories(/usr/lib/jvm/java-8-openjdk/include
		/usr/lib/jvm/java-8-openjdk/include/linux)

#network
add_library(mcres_license_client_network src/network/network.c src/network/network.h src/network/packet.c src/network/packet.h)
#crypto
add_library(mcres_license_client_crypto src/crypto/ecc.c src/crypto/ecc.h src/crypto/blowfish.h src/crypto/idea.h src/crypto/idea.c src/crypto/base64.h src/crypto/base64.c src/crypto/blowfish.c)
#class analysis
add_library(mcres_license_client_class src/class/ClassHeader.h src/class/ClassAnalyzer.c)
#lifecycle
add_library(mcres_license_client_lifecycle src/lifecycle/lifecycle.h src/lifecycle/lifecycle.c src/lifecycle/hardware/hardware.h src/lifecycle/hardware/hardware.c)
#util
add_library(mcres_license_client_util src/util/time.c src/util/util.h src/util/java.c src/util/memory.c)
#debug
add_library(mcres_license_client_debug src/debug/debug.h src/debug/debug.c)
#core
add_library(mcres_license_client SHARED src/main/decypher.c src/entry.c src/main/main.h src/com_mcres_luckyfish_angustifolia_licenseclient_plugin_classloader_EncryptedClassLoader.h)

set_target_properties(mcres_license_client PROPERTIES PUBLIC_HEADER src/com_mcres_luckyfish_angustifolia_licenseclient_plugin_classloader_EncryptedClassLoader.h)

add_dependencies(mcres_license_client mcres_license_client_network)
add_dependencies(mcres_license_client_network mcres_license_client_util)
add_dependencies(mcres_license_client mcres_license_client_crypto)
add_dependencies(mcres_license_client mcres_license_client_lifecycle)
add_dependencies(mcres_license_client_lifecycle mcres_license_client_crypto)
add_dependencies(mcres_license_client mcres_license_client_class)
add_dependencies(mcres_license_client_class mcres_license_client_util)
add_dependencies(mcres_license_client mcres_license_client_util)
add_dependencies(mcres_license_client_network mcres_license_client_crypto)
add_dependencies(mcres_license_client mcres_license_client_debug)

target_link_libraries(mcres_license_client_crypto openssl)
target_link_libraries(mcres_license_client_crypto openssl_ssl)
target_link_libraries(mcres_license_client_class mcres_license_client_util)
target_link_libraries(mcres_license_client mcres_license_client_debug)
target_link_libraries(mcres_license_client mcres_license_client_util)
target_link_libraries(mcres_license_client mcres_license_client_network)
target_link_libraries(mcres_license_client mcres_license_client_crypto)
target_link_libraries(mcres_license_client mcres_license_client_lifecycle)
target_link_libraries(mcres_license_client mcres_license_client_class)
