#set flags

if (UNIX)
    SET(COMMON_CXX_FLAGS
    -Wall
    -Wconversion
    -Werror
    -Wextra
    -Winit-self
    -Wold-style-cast
    -Woverloaded-virtual
    -Wshadow
    -Wsign-promo
    -Wunreachable-code
    -pedantic
    -std=c++17)

    ADD_COMPILE_OPTIONS(
            "$<$<COMPILE_LANGUAGE:CXX>:${COMMON_CXX_FLAGS}>"
    )
endif (UNIX)
