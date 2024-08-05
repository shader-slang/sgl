if (VCPKG_TARGET_IS_WINDOWS)
	if (VCPKG_TARGET_ARCHITECTURE MATCHES "x64")
		vcpkg_download_distfile(
			ARCHIVE
			URLS "https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/${VERSION}"
			FILENAME "agility-sdk-${VERSION}.zip"
			SHA512 97e3ec5fa2897b51156da6b78077e3f5451f639431175fd0dd4deb66875d941f75c21b38264ac15df2c446be5e19cbb7933ae4a4df2042f2567d7c6b70572ca5
        )

		vcpkg_extract_source_archive(
			BINDIST_PATH
			ARCHIVE "${ARCHIVE}"
			NO_REMOVE_ONE_LEVEL
		)

		file(GLOB DYNLIBS "${BINDIST_PATH}/build/native/bin/x64/*.dll")
		file(INSTALL ${DYNLIBS} DESTINATION "${CURRENT_PACKAGES_DIR}/bin")
		file(INSTALL ${DYNLIBS} DESTINATION "${CURRENT_PACKAGES_DIR}/debug/bin")

		file(GLOB EXES "${BINDIST_PATH}/build/native/bin/x64/*.exe")
		file(INSTALL ${DYNLIBS} DESTINATION "${CURRENT_PACKAGES_DIR}/tools/${PORT}")

		file(GLOB HEADERS "${BINDIST_PATH}/build/native/include/*.h")
		file(INSTALL ${HEADERS} DESTINATION "${CURRENT_PACKAGES_DIR}/include")

		vcpkg_install_copyright(FILE_LIST "${BINDIST_PATH}/LICENSE.txt")

	endif()
endif()
