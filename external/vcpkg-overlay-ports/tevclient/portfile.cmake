vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO westlicht/tevclient
    REF 680ef5af0f137167c519a3707deff86ac82fd21d
    SHA512 359ec00f125805daf3710750c5879d2d378660be58d1c386fa9690e1cd57fb4699d1db32b7ee00c36d326fc68da1120951e5ac1d1b9310672440a18cf8e6058e
    HEAD_REF master
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
 )

vcpkg_cmake_install()
vcpkg_copy_pdbs()

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
