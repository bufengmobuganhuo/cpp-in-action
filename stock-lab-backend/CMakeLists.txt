cmake_minimum_required(VERSION 3.5)
project(stock-lab-backend CXX)

include(CheckIncludeFileCXX)

check_include_file_cxx(any HAS_ANY)
check_include_file_cxx(string_view HAS_STRING_VIEW)
check_include_file_cxx(coroutine HAS_COROUTINE)
if (NOT "${CMAKE_CXX_STANDARD}" STREQUAL "")
    # Do nothing
elseif (HAS_ANY AND HAS_STRING_VIEW AND HAS_COROUTINE)
    set(CMAKE_CXX_STANDARD 20)
elseif (HAS_ANY AND HAS_STRING_VIEW)
    set(CMAKE_CXX_STANDARD 17)
else ()
    set(CMAKE_CXX_STANDARD 14)
endif ()

set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(${PROJECT_NAME} main.cc
        dots/LoginDto.h
        dots/JsonResult.h
        dots/ResultCode.h
        services/AuthService.cpp
        services/AuthService.h
        repositories/EmailWhitelistRepository.cpp
        repositories/EmailWhitelistRepository.h
        utils/TtlCache.h
        services/MailService.cpp
        services/MailService.h
        utils/JwtUtils.h
        utils/Snowflake.h
        repositories/UserRepository.cpp
        repositories/UserRepository.h
        dots/loginRespDto.h
        services/ContractService.cpp
        services/ContractService.h
        dots/ContractSearchResp.h
        dots/Contract.h
        services/PositionSnapshotService.cpp
        services/PositionSnapshotService.h
        repositories/PositionSnapshotRepository.cpp
        repositories/PositionSnapshotRepository.h
        enums/PositionStatus.h
        repositories/TransactionRecordRepository.cpp
        repositories/TransactionRecordRepository.h
        vo/PositionSnapshotDetailVo.h
        vo/TransactionPlanVo.h
        utils/Math.h
        services/QuoteService.cpp
        services/QuoteService.h
        dots/Quote.h
        services/TransactionPlanService.cpp
        services/TransactionPlanService.h
        dots/TransactionPlanDto.h
        enums/TransactionPlanStatus.h
        repositories/TransactionPlanRepository.cpp
        repositories/TransactionPlanRepository.h
        dots/PageResult.h
        repositories/BaseRepository.h
        utils/CronUtil.h
        utils/CronUtil.cpp
        services/SchedulerService.cpp
        services/SchedulerService.h
        services/TransactionRecordService.cpp
        services/TransactionRecordService.h
        dots/TransactionStatisticsVo.h
        controllers/BaseCtrl.cpp
        controllers/BaseCtrl.h
        enums/TransactionPlanFrequency.h
)

# ##############################################################################
# If you include the drogon source code locally in your project, use this method
# to add drogon 
# add_subdirectory(drogon) 
# target_link_libraries(${PROJECT_NAME} PRIVATE drogon)
#
# and comment out the following lines
find_package(Drogon CONFIG REQUIRED)
find_package(jwt-cpp CONFIG REQUIRED)
find_package(Boost REQUIRED)
find_package(croncpp CONFIG REQUIRED)
target_link_libraries(${PROJECT_NAME} PRIVATE croncpp::croncpp)
target_link_libraries(${PROJECT_NAME} PRIVATE Drogon::Drogon)
target_link_libraries(${PROJECT_NAME} PRIVATE jwt-cpp::jwt-cpp)
target_link_libraries(${PROJECT_NAME} PRIVATE Boost::headers)

# ##############################################################################

if (CMAKE_CXX_STANDARD LESS 17)
    message(FATAL_ERROR "c++17 or higher is required")
elseif (CMAKE_CXX_STANDARD LESS 20)
    message(STATUS "use c++17")
else ()
    message(STATUS "use c++20")
endif ()

aux_source_directory(controllers CTL_SRC)
aux_source_directory(filters FILTER_SRC)
aux_source_directory(plugins PLUGIN_SRC)
aux_source_directory(models MODEL_SRC)

drogon_create_views(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/views
                    ${CMAKE_CURRENT_BINARY_DIR})
# use the following line to create views with namespaces.
# drogon_create_views(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/views
#                     ${CMAKE_CURRENT_BINARY_DIR} TRUE)
# use the following line to create views with namespace CHANGE_ME prefixed
# and path namespaces.
# drogon_create_views(${PROJECT_NAME} ${CMAKE_CURRENT_SOURCE_DIR}/views
#                     ${CMAKE_CURRENT_BINARY_DIR} TRUE CHANGE_ME)

target_include_directories(${PROJECT_NAME}
                           PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}
                                   ${CMAKE_CURRENT_SOURCE_DIR}/models)
target_sources(${PROJECT_NAME}
               PRIVATE
               ${SRC_DIR}
               ${CTL_SRC}
               ${FILTER_SRC}
               ${PLUGIN_SRC}
               ${MODEL_SRC})
# ##############################################################################
# uncomment the following line for dynamically loading views 
# set_property(TARGET ${PROJECT_NAME} PROPERTY ENABLE_EXPORTS ON)

# ##############################################################################

add_subdirectory(test)
