include_directories(
    ../lib/include/
    ${Boost_INCLUDE_DIRS}
)

add_executable(puppetcpp
    main.cc
)
target_link_libraries(puppetcpp puppet ${Boost_LIBRARIES})
set_target_properties(puppetcpp PROPERTIES COTIRE_UNITY_LINK_LIBRARIES_INIT "COPY_UNITY")

install(TARGETS puppetcpp DESTINATION bin)

