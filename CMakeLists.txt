cmake_minimum_required(VERSION 3.0.0)

project(SEARCH_ENGINE)

include_directories(include)

aux_source_directory(src/server/offline/WordDict DICT)

aux_source_directory(src/server/offline/KeyWord KEY)

aux_source_directory(src/server/online/TcpServer TCPSERVER)

set(CONFIG src/server/Configuration.cc)

add_compile_options(-Wall -g)

add_executable(wordDict ${DICT} ${CONFIG}  testWord.cc)

add_executable(keyWord ${KEY} ${TCPSERVER} ${CONFIG} testKey.cc)

# set_target_properties(wordDict PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ../bin
# )

# set_target_properties(keyWord PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ../bin
# )
#add_executable(server )
#add_executable(client )
