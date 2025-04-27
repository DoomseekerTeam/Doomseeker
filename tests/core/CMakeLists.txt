find_package(${Qt_PACKAGE} REQUIRED COMPONENTS Test)

add_custom_target(tests)

##
# Create Doomseeker's test target from test's source file.
#
# The name of the test is not specified directly. Instead, the function
# derives it from the name and path of the test's main source file. All
# Doomseeker test targets are prefixed with 'doomseeker_test_'
#
# The function establishes some common build properties for Qt
# and also links the test with Doomseeker's main source code.
#
# All tests are brought under an umbrella 'tests' target, so you
# can build them all by just building 'tests'.
#
# @param main_source The main source code of the test.
# @param ARGN Additional source code files for the test.
function(doomseeker_add_test main_source)
	# Base the name of the test on its main source file.
	get_filename_component(source_wle "${main_source}" NAME_WLE)
	string(REGEX REPLACE "^test_" "" test_basename "${source_wle}")

	get_filename_component(source_dir "${main_source}" DIRECTORY)
	if (source_dir)
		# Target names must adhere to CMP0037
		# https://cmake.org/cmake/help/v3.5/policy/CMP0037.html
		string(REPLACE "/" "_" dir_prefix "${source_dir}")
		set(test_basename "${dir_prefix}_${test_basename}")
	endif()
	set(test_name "doomseeker_test_${test_basename}")

	# Collect the main source and additional files in one variable.
	list(APPEND sources "${main_source}")
	if (ARGN)
		list(APPEND sources ${ARGN})
	endif()

	# Create the test's executable target.
	add_executable("${test_name}" ${sources})
	set_target_properties("${test_name}" PROPERTIES AUTOMOC ON AUTOUIC ON AUTORCC ON)
	target_link_libraries("${test_name}"
		PRIVATE
		doomseeker_obj
		${Qt_PACKAGE}::Test
	)

	# Register the test.
	add_test(NAME "${test_name}" COMMAND "${test_name}")

	# Add the test to the umbrella 'tests' target.
	add_dependencies(tests "${test_name}")
endfunction()

doomseeker_add_test(test_datetime.cpp)
