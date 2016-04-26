include_directories(
    ../lib/include/
    ${Boost_INCLUDE_DIRS}
    ${Onigmo_INCLUDE_DIRS}
)

add_executable(puppetcpp
    main.cc
)
target_link_libraries(puppetcpp puppet ${Boost_LIBRARIES} ${Onigmo_LIBRARIES})

install(TARGETS puppetcpp DESTINATION bin)

cotire(puppetcpp)
