cmake_minimum_required(VERSION 3.10)

project(Transport_Catalogue CXX)
set(CMAKE_CXX_STANDARD 17)
list(APPEND CMAKE_PREFIX_PATH "./Protobuf")
find_package(Protobuf REQUIRED)

# (Под *nix может понадобиться дополнительный пакет Threads и соответствующий флаг линковщику).
find_package(Threads REQUIRED)

protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS transport_catalogue.proto)

set(CAT_FILES domain.h geo.h graph.h json_builder.h json_reader.h json.h map_renderer.h ranges.h request_handler.h router.h serialization.h svg.h transport_catalogue.h transport_router.h main.cpp domain.cpp geo.cpp json_builder.cpp json_reader.cpp json.cpp map_renderer.cpp request_handler.cpp 
serialization.cpp svg.cpp transport_catalogue.cpp transport_router.cpp transport_catalogue.proto)
# set(CAT_FILES domain.h geo.h graph.h input_reader.h json_builder.h json_reader.h json.h log_duration.h map_renderer.h ranges.h request_handler.h router.h serialization.h stat_reader.h svg.h transport_catalogue.h transport_router.h main.cpp domain.cpp geo.cpp input_reader.cpp json_builder.cpp json_reader.cpp json.cpp map_renderer.cpp request_handler.cpp 
# serialization.cpp stat_reader.cpp svg.cpp transport_catalogue.cpp transport_router.cpp transport_catalogue.proto)

add_executable(transport_catalogue ${PROTO_SRCS} ${PROTO_HDRS} ${CAT_FILES})
target_include_directories(transport_catalogue PUBLIC ${Protobuf_INCLUDE_DIRS})
target_include_directories(transport_catalogue PUBLIC ${CMAKE_CURRENT_BINARY_DIR})

string(REPLACE "protobuf.lib" "protobufd.lib" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")
string(REPLACE "protobuf.a" "protobufd.a" "Protobuf_LIBRARY_DEBUG" "${Protobuf_LIBRARY_DEBUG}")

target_link_libraries(transport_catalogue "$<IF:$<CONFIG:Debug>,${Protobuf_LIBRARY_DEBUG},${Protobuf_LIBRARY}>" Threads::Threads)