# Sanitizer integration for ohtoai-webhook
# Usage: cmake -B build -DENABLE_SANITIZERS=ON

option(ENABLE_SANITIZERS "Enable sanitizers (ASan, UBSan)" OFF)

if(ENABLE_SANITIZERS)
    if(MSVC)
        # MSVC Address Sanitizer
        set(SANITIZER_FLAGS "/fsanitize=address")
        message(STATUS "Sanitizers: ASan enabled (MSVC)")
    else()
        # GCC/Clang sanitizers
        set(SANITIZER_FLAGS
            -fsanitize=address
            -fsanitize=undefined
            -fsanitize=leak
            -fno-omit-frame-pointer
        )
        message(STATUS "Sanitizers: ASan, UBSan, LSan enabled")
    endif()

    # Apply to all targets in the directory
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${SANITIZER_FLAGS}")
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${SANITIZER_FLAGS}")
endif()
