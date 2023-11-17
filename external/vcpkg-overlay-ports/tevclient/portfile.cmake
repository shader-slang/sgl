vcpkg_from_github(
    OUT_SOURCE_PATH SOURCE_PATH
    REPO westlicht/tevclient
    REF ee2705577be65dba0c4416bb7a5dc05cbdb137ad
    SHA512 63a8a6e354048107b8f431106dae100fe843f0c9f3c8b1fe5ec3876bb22ea9e6a44d3a506e7e2b171f51529f4e3a8f09745b40e1f0b5ba265f1f8a39c0807b06
    HEAD_REF master
)

vcpkg_cmake_configure(
    SOURCE_PATH "${SOURCE_PATH}"
 )

vcpkg_cmake_install()
vcpkg_copy_pdbs()

vcpkg_cmake_config_fixup(CONFIG_PATH share/cmake/tevclient)

file(REMOVE_RECURSE "${CURRENT_PACKAGES_DIR}/debug/include")

vcpkg_install_copyright(FILE_LIST "${SOURCE_PATH}/LICENSE")
