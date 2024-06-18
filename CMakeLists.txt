cmake_minimum_required(VERSION 3.0.0)

project(SEARCH_ENGINE)

include_directories(include)

aux_source_directory(src/server/offline/WordDict DICT)

aux_source_directory(src/server/offline/KeyWord KEY)

aux_source_directory(src/server/offline/PageLib OLDPAGE)

aux_source_directory(src/server/online/TcpServer TCPSERVER)

set(CONFIG src/server/Configuration.cc)
set(PROTOCOL src/TransProtocol.cc)
set(WORDCUT src/server/offline/WordDict/WordSplit_JieBa.cc)
set(LOG src/server/MyLog.cc)



add_compile_options(-Wall -g -O0 -lm)

add_executable(wordDict ${DICT} ${CONFIG} testWord.cc)

add_executable(keyWord ${KEY} ${TCPSERVER} ${PROTOCOL} ${CONFIG} ${LOG} testKey.cc)

add_executable(pagelib ${OLDPAGE} ${DICT} ${CONFIG} ${LOG} testPageLib.cc)

add_executable(queryPage ${OLDPAGE} ${WORDCUT} ${TCPSERVER} ${PROTOCOL} ${CONFIG} ${LOG} testPageQuery.cc)

add_executable(server ${KEY} ${OLDPAGE} ${WORDCUT} ${TCPSERVER} ${PROTOCOL} ${CONFIG} ${LOG} testQuery.cc)

add_executable(redisServer ${KEY} ${OLDPAGE} ${WORDCUT} ${TCPSERVER} ${PROTOCOL} ${CONFIG} ${LOG} testRedisQuery.cc)

find_library(HIREDIS_LIB hiredis)
target_link_libraries(redisServer PUBLIC ${HIREDIS_LIB})
target_link_libraries(pagelib PUBLIC ${HIREDIS_LIB})
target_link_libraries(queryPage PUBLIC ${HIREDIS_LIB})
target_link_libraries(server PUBLIC ${HIREDIS_LIB})

find_library(REDIS_PLUS_PLUS_LIB redis++)
target_link_libraries(redisServer PUBLIC ${REDIS_PLUS_PLUS_LIB})
target_link_libraries(pagelib PUBLIC ${REDIS_PLUS_PLUS_LIB})
target_link_libraries(queryPage PUBLIC ${REDIS_PLUS_PLUS_LIB})
target_link_libraries(server PUBLIC ${REDIS_PLUS_PLUS_LIB})

find_library(LOG4CPP log4cpp)
target_link_libraries(keyWord PUBLIC ${LOG4CPP})
target_link_libraries(redisServer PUBLIC ${LOG4CPP})
target_link_libraries(pagelib PUBLIC ${LOG4CPP})
target_link_libraries(queryPage PUBLIC ${LOG4CPP})
target_link_libraries(server PUBLIC ${LOG4CPP})

add_executable(client ${PROTOCOL} src/client/client.cc)
# set_target_properties(wordDict PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ../bin
# )

# set_target_properties(keyWord PROPERTIES
#     RUNTIME_OUTPUT_DIRECTORY ../bin
# )
#add_executable(server )
#add_executable(client )
