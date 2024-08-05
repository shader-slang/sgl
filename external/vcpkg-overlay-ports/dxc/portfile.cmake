set(TAG "2023_08_14")

if (VCPKG_TARGET_IS_WINDOWS)
	if (VCPKG_TARGET_ARCHITECTURE MATCHES "x64")
		vcpkg_download_distfile(
			ARCHIVE
			URLS "https://github.com/microsoft/DirectXShaderCompiler/releases/download/v${VERSION}/dxc_${TAG}.zip"
			FILENAME "dxc-${VERSION}-win64.zip"
            SHA512 3bc49f77b55f58de88002a75b38e5acdb8600b0b73729320a25a27af08f1f21d0b4aec92ee9d736eb30bae42f4e0f2f32d25d6635fa71aedcaf82440e6d2433e
        )

		vcpkg_extract_source_archive(
			BINDIST_PATH
			ARCHIVE "${ARCHIVE}"
			NO_REMOVE_ONE_LEVEL
		)

		file(GLOB DYNLIBS "${BINDIST_PATH}/bin/x64/*.dll")
		file(INSTALL ${DYNLIBS} DESTINATION "${CURRENT_PACKAGES_DIR}/bin")
		file(INSTALL ${DYNLIBS} DESTINATION "${CURRENT_PACKAGES_DIR}/debug/bin")

		# file(GLOB LIBS "${BINDIST_PATH}/lib/x64/*.lib")
		# file(INSTALL ${LIBS} DESTINATION "${CURRENT_PACKAGES_DIR}/lib")
		# file(INSTALL ${LIBS} DESTINATION "${CURRENT_PACKAGES_DIR}/debug/lib")

		# file(GLOB HEADERS "${BINDIST_PATH}/inc/*.h")
		# file(INSTALL ${HEADERS} DESTINATION "${CURRENT_PACKAGES_DIR}/include")

		# vcpkg_install_copyright(FILE_LIST "${BINDIST_PATH}/LICENSE-LLVM.txt" "${BINDIST_PATH}/LICENSE-MIT.txt" "${BINDIST_PATH}/LICENSE-MS.txt")

	endif()
endif()
