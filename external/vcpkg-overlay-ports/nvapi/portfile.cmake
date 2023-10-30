set(COMMIT "89b4365")

if (VCPKG_TARGET_IS_WINDOWS)
	if (VCPKG_TARGET_ARCHITECTURE MATCHES "x64")
		vcpkg_download_distfile(
			ARCHIVE
			URLS "https://github.com/NVIDIA/nvapi/archive/${COMMIT}.zip"
			FILENAME "nvapi-${VERSION}-win64.zip"
            SHA512 fe50033a61c0e78db7f60e9b5c8255ecc18e63060ea486e2fc4569fb465ef4ddc6cf9e93a3c6be0e66c50b26eda1625874d00e0febfc964c8716cfd351c4f3fc
        )

		vcpkg_extract_source_archive(
			BINDIST_PATH
			ARCHIVE "${ARCHIVE}"
		)

		file(GLOB LIBS "${BINDIST_PATH}/amd64/*.lib")
		file(INSTALL ${LIBS} DESTINATION "${CURRENT_PACKAGES_DIR}/lib")
		file(INSTALL ${LIBS} DESTINATION "${CURRENT_PACKAGES_DIR}/debug/lib")

		file(GLOB HEADERS "${BINDIST_PATH}/nvapi*.h" "${BINDIST_PATH}/nv*.h")
		file(INSTALL ${HEADERS} DESTINATION "${CURRENT_PACKAGES_DIR}/include")

		vcpkg_install_copyright(FILE_LIST "${BINDIST_PATH}/License.txt")

	endif()
endif()
