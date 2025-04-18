# this Makefile serves as a simple runner for some of common (for me) CMake operations

CXX=g++
# MUST have the same value as in CMakelists.txt
PROGRAM_NAME := APO

LINUX_BUILD_DIR := build
WINDOWS_BUILD_DIR := build-windows

LINUX_DEBUG_BUILD_DIR := ${LINUX_BUILD_DIR}/Debug
LINUX_RELEASE_BUILD_DIR := ${LINUX_BUILD_DIR}/Release

WINDOWS_DEBUG_BUILD_DIR := ${WINDOWS_BUILD_DIR}/Debug
WINDOWS_RELEASE_BUILD_DIR := ${WINDOWS_BUILD_DIR}/Release
WINDOWS_PACKAGE_DIR := ${WINDOWS_BUILD_DIR}/package
WINDOWS_PACKAGE_ZIP := ${WINDOWS_PACKAGE_DIR}/${PROGRAM_NAME}.zip

# TARGETS:
# - linux-debug
# - linux-release
# - windows-debug
# - windows-release
# - windows-package
# - tests


linux-debug: ${LINUX_DEBUG_BUILD_DIR}
	CXX=${CXX} cmake --build ${LINUX_DEBUG_BUILD_DIR} --parallel $(nproc)

linux-release: ${LINUX_RELEASE_BUILD_DIR}
	CXX=${CXX} cmake --build ${LINUX_RELEASE_BUILD_DIR} --parallel $(nproc)

${LINUX_DEBUG_BUILD_DIR}:
	CXX=clang++ cmake -B${LINUX_DEBUG_BUILD_DIR} -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=YES
	@cp ${LINUX_DEBUG_BUILD_DIR}/compile_commands.json ${LINUX_BUILD_DIR}/

${LINUX_RELEASE_BUILD_DIR}:
	CXX=${CXX} cmake -B${LINUX_RELEASE_BUILD_DIR} -DCMAKE_BUILD_TYPE=Release

tests: ${LINUX_DEBUG_BUILD_DIR}/CTestTestfile.cmake
	cmake --build ${LINUX_DEBUG_BUILD_DIR} --parallel $(nproc)
	cd ${LINUX_DEBUG_BUILD_DIR} && ctest --output-on-failure

${LINUX_DEBUG_BUILD_DIR}/CTestTestfile.cmake:
	cmake -B${LINUX_DEBUG_BUILD_DIR} -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON

windows-debug: ${WINDOWS_DEBUG_BUILD_DIR}
	cmake --build ${WINDOWS_DEBUG_BUILD_DIR} --parallel $(nproc)

windows-release: ${WINDOWS_RELEASE_BUILD_DIR}
	cmake --build ${WINDOWS_RELEASE_BUILD_DIR} --parallel $(nproc)

windows-package: ${WINDOWS_PACKAGE_ZIP}

${WINDOWS_DEBUG_BUILD_DIR}:
	cmake -B${WINDOWS_DEBUG_BUILD_DIR} -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=./toolchain-mingw.cmake

${WINDOWS_RELEASE_BUILD_DIR}:
	cmake -B${WINDOWS_RELEASE_BUILD_DIR} -DCMAKE_BUILD_TYPE=Release -DCMAKE_TOOLCHAIN_FILE=./toolchain-mingw.cmake

${WINDOWS_PACKAGE_ZIP}: windows-release
	mkdir -p ${WINDOWS_PACKAGE_DIR}
	cp ${WINDOWS_RELEASE_BUILD_DIR}/${PROGRAM_NAME}.exe ${WINDOWS_PACKAGE_DIR}/
	cp /opt/windows-opencv-build/bin/*.dll ${WINDOWS_PACKAGE_DIR}/
	wine /opt/Qt/6.8.2/mingw_64/bin/windeployqt.exe --release ${WINDOWS_PACKAGE_DIR}/${PROGRAM_NAME}.exe
	# zip -r ${WINDOWS_PACKAGE_DIR}/${PROGRAM_NAME}-windows.zip ${WINDOWS_PACKAGE_DIR}/

clean:
	rm -rf ${LINUX_BUILD_DIR} ${WINDOWS_BUILD_DIR}

.PHONY: linux-debug linux-release tests windows-debug windows-release windows-package clean
