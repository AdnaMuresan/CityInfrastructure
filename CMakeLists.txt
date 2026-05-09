cmake_minimum_required(VERSION 4.2)
project(CityInfrastructure C)

set(CMAKE_C_STANDARD 99)

add_executable(CityInfrastructure main.c
        main.c
        main.c
        models.h
        fs_utils.h
        fs_utils.c
        reports.h
        reports.c
        filter.h
        filter.c
        monitor_reports.c
        city_hub.c
        scorer.c
)
