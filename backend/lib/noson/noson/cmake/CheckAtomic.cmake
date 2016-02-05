include(CheckCXXSourceCompiles)

check_library_exists(atomic __atomic_fetch_add_4 "" HAVE_LIBATOMIC)
if (HAVE_LIBATOMIC)
  list(APPEND CMAKE_REQUIRED_LIBRARIES "atomic")
endif()

CHECK_CXX_SOURCE_COMPILES("int main() { long* temp=0; long ret=__sync_add_and_fetch(temp, 1); return 0; }" HAS_BUILTIN_SYNC_ADD_AND_FETCH)
CHECK_CXX_SOURCE_COMPILES("int main() { long* temp=0; long ret=__sync_sub_and_fetch(temp, 1); return 0; }" HAS_BUILTIN_SYNC_SUB_AND_FETCH)
CHECK_CXX_SOURCE_COMPILES("int main() { long *temp = 0; long ret=__sync_val_compare_and_swap(temp, 1, 1); return 0; }" HAS_BUILTIN_SYNC_VAL_COMPARE_AND_SWAP)