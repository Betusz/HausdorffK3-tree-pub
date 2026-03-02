# --- DESCARGA DE GOOGLE TEST ---
include(FetchContent)

# Para Windows/MSVC: Forzar el uso del CRT compartido
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)

FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG    v1.14.0
)
FetchContent_MakeAvailable(googletest)

# --- EJECUTABLE DE PRUEBAS ---
# Usamos CONFIGURE_DEPENDS para detectar nuevos archivos de test automáticamente
file(GLOB_RECURSE TEST_SOURCES CONFIGURE_DEPENDS "*.cpp")
add_executable(run_tests ${TEST_SOURCES})

# --- ENLACES ---
target_link_libraries(run_tests PRIVATE hausdorff_lib GTest::gtest_main)

# --- INTEGRACIÓN CON CTEST ---
include(GoogleTest)
gtest_discover_tests(run_tests)

