if (VCPKG_TARGET_IS_WINDOWS)
	if (VCPKG_TARGET_ARCHITECTURE MATCHES "x64")
		vcpkg_download_distfile(
			ARCHIVE
			URLS "https://www.nuget.org/api/v2/package/Microsoft.Direct3D.D3D12/1.611.5"
			FILENAME "agility-sdk-${VERSION}.zip"
            SKIP_SHA512
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
