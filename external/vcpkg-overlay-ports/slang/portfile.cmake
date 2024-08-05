if (VCPKG_TARGET_IS_WINDOWS)
	set(SLANG_EXE_SUFFIX ".exe")
	set(SLANG_LIB_PREFIX "")
	set(SLANG_LIB_SUFFIX ".lib")
	set(SLANG_DYNLIB_SUFFIX ".dll")
	if (VCPKG_TARGET_ARCHITECTURE MATCHES "x64")
		vcpkg_download_distfile(
			ARCHIVE
			URLS "https://github.com/shader-slang/slang/releases/download/v${VERSION}/slang-${VERSION}-win64.zip"
			FILENAME "slang-${VERSION}-win64.zip"
            SKIP_SHA512
        )
		set(SLANG_BIN_PATH "bin/windows-x64/release")
	elseif (VCPKG_TARGET_ARCHITECTURE MATCHES "x86")
		vcpkg_download_distfile(
			ARCHIVE
			URLS "https://github.com/shader-slang/slang/releases/download/v${VERSION}/slang-${VERSION}-win32.zip"
			FILENAME "slang-${VERSION}-win32.zip"
            SKIP_SHA512
		)
		set(SLANG_BIN_PATH "bin/windows-x86/release")
	else()
		message(FATAL_ERROR "Unsupported platform. Please implement me!")
	endif()
elseif (VCPKG_TARGET_IS_OSX)
	set(SLANG_EXE_SUFFIX "")
	set(SLANG_LIB_PREFIX "lib")
	set(SLANG_LIB_SUFFIX ".a")
	set(SLANG_DYNLIB_SUFFIX ".dylib")
	if (VCPKG_TARGET_ARCHITECTURE MATCHES "x64")
		vcpkg_download_distfile(
			ARCHIVE
			URLS "https://github.com/shader-slang/slang/releases/download/v${VERSION}/slang-${VERSION}-macos-x64.zip"
			FILENAME "slang-${VERSION}-macos-x64.zip"
            SKIP_SHA512
		)
		set(SLANG_BIN_PATH "bin/macosx-x64/release")
	elseif (VCPKG_TARGET_ARCHITECTURE MATCHES "arm64")
		vcpkg_download_distfile(
			ARCHIVE
			URLS "https://github.com/shader-slang/slang/releases/download/v${VERSION}/slang-${VERSION}-macos-aarch64.zip"
			FILENAME "slang-${VERSION}-macos-aarch64.zip"
            SKIP_SHA512
		)
		set(SLANG_BIN_PATH "bin/macosx-aarch64/release")
	else()
		message(FATAL_ERROR "Unsupported platform. Please implement me!")
	endif()
elseif(VCPKG_TARGET_IS_LINUX)
	set(SLANG_EXE_SUFFIX "")
	set(SLANG_LIB_PREFIX "lib")
	set(SLANG_LIB_SUFFIX ".a")
	set(SLANG_DYNLIB_SUFFIX ".so")
	if (VCPKG_TARGET_ARCHITECTURE MATCHES "x64")
		vcpkg_download_distfile(
			ARCHIVE
			URLS "https://github.com/shader-slang/slang/releases/download/v${VERSION}/slang-${VERSION}-linux-x86_64.tar.gz"
			FILENAME "slang-${VERSION}-linux-x86_64.tar.gz"
            SKIP_SHA512
		)
		set(SLANG_BIN_PATH "bin/linux-x64/release")
	else()
		message(FATAL_ERROR "Unsupported platform. Please implement me!")
	endif()
else()
	message(FATAL_ERROR "Unsupported platform. Please implement me!")
endif()

vcpkg_extract_source_archive(
	BINDIST_PATH
	ARCHIVE "${ARCHIVE}"
	NO_REMOVE_ONE_LEVEL
)

file(GLOB DYNLIBS "${BINDIST_PATH}/${SLANG_BIN_PATH}/*${SLANG_DYNLIB_SUFFIX}")
file(INSTALL ${DYNLIBS} DESTINATION "${CURRENT_PACKAGES_DIR}/bin")
file(INSTALL ${DYNLIBS} DESTINATION "${CURRENT_PACKAGES_DIR}/debug/bin")

file(GLOB LIBS "${BINDIST_PATH}/${SLANG_BIN_PATH}/*${SLANG_LIB_SUFFIX}")
file(INSTALL ${LIBS} DESTINATION "${CURRENT_PACKAGES_DIR}/lib")
file(INSTALL ${LIBS} DESTINATION "${CURRENT_PACKAGES_DIR}/debug/lib")

file(GLOB EXES "${BINDIST_PATH}/${SLANG_BIN_PATH}/*${SLANG_EXE_SUFFIX}")
file(INSTALL ${EXES} DESTINATION "${CURRENT_PACKAGES_DIR}/tools/${PORT}")

file(GLOB HEADERS "${BINDIST_PATH}/*.h")
file(INSTALL ${HEADERS} DESTINATION "${CURRENT_PACKAGES_DIR}/include")

# vcpkg_from_github(
# 	OUT_SOURCE_PATH SOURCE_PATH
# 	REPO shader-slang/slang
# 	REF v${VERSION}
#   SHA512
# 	HEAD_REF master
# )

vcpkg_install_copyright(FILE_LIST "${CMAKE_CURRENT_LIST_DIR}/LICENSE")
