include_directories(
    ../lib/include/
    ${Boost_INCLUDE_DIRS}
)

add_executable(puppetcpp
    main.cc
)
target_link_libraries(puppetcpp puppet ${Boost_LIBRARIES})

install(TARGETS puppetcpp DESTINATION bin)

cotire(puppetcpp)
