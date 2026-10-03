cmake_minimum_required(VERSION 3.16)

project(academic_scheduler LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

add_executable(main
    main.cpp
    src/application.cpp
    src/data_loader.cpp
    src/utilities.cpp
)
