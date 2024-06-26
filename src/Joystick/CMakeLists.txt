find_package(Qt6 REQUIRED COMPONENTS Core Qml)

qt_add_library(Joystick STATIC
	Joystick.cc
	Joystick.h
	JoystickManager.cc
	JoystickManager.h
)

if(ANDROID)
	target_sources(Joystick
		PRIVATE
			JoystickAndroid.cc
			JoystickAndroid.h
	)

	target_link_libraries(Joystick PRIVATE Qt6::CorePrivate)
else()
	include(BuildSdl2)
	build_sdl2()
	if(SDL2_FOUND)
		message(STATUS "Building JoystickSDL")
		target_sources(Joystick
			PRIVATE
				JoystickSDL.cc
				JoystickSDL.h
		)
    	target_link_libraries(Joystick PRIVATE SDL2::SDL2)
	endif()
endif()

target_link_libraries(Joystick
	PRIVATE
		Qt6::Qml
		AutoPilotPlugins
		Camera
        FirmwarePlugin
		qgc
		Settings
		Utilities
		Vehicle
		VideoManager
	PUBLIC
		Qt6::Core
		comm
		QmlControls
)

target_include_directories(Joystick PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
