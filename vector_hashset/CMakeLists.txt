cmake_minimum_required(VERSION 3.10.0)
project(vector_hashset VERSION 0.1.0 LANGUAGES C)

#add_executable(vector_hashset vector.c vectortest.c)

add_executable(test_hashset hashset.c hashsettest.c vector.c)
