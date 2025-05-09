# CMakeLists.
#
# Author: Steffen Vogel <post@steffenvogel.de>
# SPDX-FileCopyrightText: 2014-2023 Institute for Automation of Complex Power Systems, RWTH Aachen University
# SPDX-License-Identifier: Apache-2.0

set(SRCS
    villas-convert
    villas-node
    villas-pipe
    villas-signal
    villas-compare
    villas-test-config
)

add_executable(villas-node villas-node.cpp)
target_link_libraries(villas-node PUBLIC villas)

add_executable(villas-test-config villas-test-config.cpp)
target_link_libraries(villas-test-config PUBLIC villas)

add_executable(villas-compare villas-compare.cpp)
target_link_libraries(villas-compare PUBLIC villas)

add_executable(villas-convert villas-convert.cpp)
target_link_libraries(villas-convert PUBLIC villas)

add_executable(villas-pipe villas-pipe.cpp)
target_link_libraries(villas-pipe PUBLIC villas Threads::Threads)

add_executable(villas-signal villas-signal.cpp)
target_link_libraries(villas-signal PUBLIC villas)

if(WITH_GRAPHVIZ)
    add_executable(villas-graph villas-graph.cpp)
    target_link_libraries(villas-graph PUBLIC villas)

    list(APPEND SRCS villas-graph)
endif()

if(WITH_WEB)
    add_executable(villas-relay villas-relay.cpp)
    target_include_directories(villas-relay PRIVATE ${LIBWEBSOCKETS_INCLUDE_DIRS} ${OPENSSL_INCLUDE_DIR})
    target_link_libraries(villas-relay PRIVATE PkgConfig::UUID villas)

    list(APPEND SRCS villas-relay)
endif()

if(WITH_CONFIG)
    add_executable(villas-conf2json villas-conf2json.cpp)
    target_link_libraries(villas-conf2json PUBLIC villas)

    list(APPEND SRCS villas-conf2json)
endif()

if(LIBZMQ_FOUND)
    add_executable(villas-zmq-keygen villas-zmq-keygen.cpp)
    target_link_libraries(villas-zmq-keygen PUBLIC villas-common PkgConfig::LIBZMQ)

    list(APPEND SRCS villas-zmq-keygen)
endif()

if(WITH_HOOKS)
    add_executable(villas-hook villas-hook.cpp)
    target_link_libraries(villas-hook PUBLIC villas)

    list(APPEND SRCS villas-hook)
endif()

install(
    TARGETS ${SRCS}
    COMPONENT bin
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)
