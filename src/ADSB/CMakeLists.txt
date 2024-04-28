find_package(Qt6 REQUIRED COMPONENTS Core Network Positioning)

qt_add_library(ADSB STATIC
        ADSBTCPLink.cc
        ADSBTCPLink.h
	ADSBVehicle.cc
	ADSBVehicle.h
	ADSBVehicleManager.cc
	ADSBVehicleManager.h
        MavlinkDecoder.cc
        MavlinkDecoder.h
        SBS1Decoder.cc
        SBS1Decoder.h
        TCPLinkController.cc
        TCPLinkController.h
)

target_link_libraries(ADSB
	PRIVATE
		Qt6::Network
		Settings
		Utilities
	PUBLIC
		Qt6::Core
		Qt6::Positioning
		qgc
		QmlControls
)

target_include_directories(ADSB PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
