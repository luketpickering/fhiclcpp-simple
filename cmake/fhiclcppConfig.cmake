if(NOT COMMAND cmessage)
  if(NOT WIN32)
    string(ASCII 27 Esc)
    set(CM_ColourReset "${Esc}[m")
    set(CM_ColourBold "${Esc}[1m")
    set(CM_Red "${Esc}[31m")
    set(CM_Green "${Esc}[32m")
    set(CM_Yellow "${Esc}[33m")
    set(CM_Blue "${Esc}[34m")
    set(CM_Magenta "${Esc}[35m")
    set(CM_Cyan "${Esc}[36m")
    set(CM_White "${Esc}[37m")
    set(CM_BoldRed "${Esc}[1;31m")
    set(CM_BoldGreen "${Esc}[1;32m")
    set(CM_BoldYellow "${Esc}[1;33m")
    set(CM_BoldBlue "${Esc}[1;34m")
    set(CM_BoldMagenta "${Esc}[1;35m")
    set(CM_BoldCyan "${Esc}[1;36m")
    set(CM_BoldWhite "${Esc}[1;37m")
  endif()

  message(STATUS "Setting up colored messages...")

  function(cmessage)
    list(GET ARGV 0 MessageType)
    if(MessageType STREQUAL FATAL_ERROR OR MessageType STREQUAL SEND_ERROR)
      list(REMOVE_AT ARGV 0)
      message(${MessageType} "${CM_BoldRed}${ARGV}${CM_ColourReset}")
    elseif(MessageType STREQUAL WARNING)
      list(REMOVE_AT ARGV 0)
      message(${MessageType} "${CM_BoldYellow}${ARGV}${CM_ColourReset}")
    elseif(MessageType STREQUAL AUTHOR_WARNING)
      list(REMOVE_AT ARGV 0)
      message(${MessageType} "${CM_BoldCyan}${ARGV}${CM_ColourReset}")
    elseif(MessageType STREQUAL STATUS)
      list(REMOVE_AT ARGV 0)
      message(${MessageType} "${CM_Green}[INFO]:${CM_ColourReset} ${ARGV}")
    elseif(MessageType STREQUAL CACHE)        
      list(REMOVE_AT ARGV 0)
      message(-- "${CM_Blue}[CACHE]:${CM_ColourReset} ${ARGV}")
    elseif(MessageType STREQUAL DEBUG)
      list(REMOVE_AT ARGV 0)
      if(BUILD_DEBUG_MSGS)
        message("${CM_Magenta}[DEBUG]:${CM_ColourReset} ${ARGV}")
      endif()
    else()
      message(${MessageType} "${CM_Green}[INFO]:${CM_ColourReset} ${ARGV}")
    endif()
  endfunction()
endif()

find_package(linedoc REQUIRED)

get_target_property(linedoc_Header_INC linedoc::Headers INTERFACE_INCLUDE_DIRECTORIES)

if(NOT "${CMAKE_PROJECT_NAME} " STREQUAL "fhiclcpp ")
  get_filename_component(fhiclcpp_CMAKE_DIR "${CMAKE_CURRENT_LIST_FILE}" PATH)
  include(${fhiclcpp_CMAKE_DIR}/fhiclcppVersion.cmake)

  find_path(fhiclcpp_INCLUDE_DIR
    NAMES fhiclcpp/ParameterSet.h
    PATHS ${fhiclcpp_CMAKE_DIR}/../include
  )

  cmessage(STATUS "fhiclcpp_INCLUDE_DIR: ${fhiclcpp_INCLUDE_DIR}")
  cmessage(STATUS "fhiclcpp_VERSION: ${fhiclcpp_VERSION}")

  include(FindPackageHandleStandardArgs)
  find_package_handle_standard_args(fhiclcpp
      REQUIRED_VARS 
        fhiclcpp_INCLUDE_DIR
      VERSION_VAR 
        fhiclcpp_VERSION
  )
  if(NOT TARGET fhiclcpp::Headers)
      add_library(fhiclcpp::Headers INTERFACE IMPORTED)
      set_target_properties(fhiclcpp::Headers PROPERTIES
          INTERFACE_INCLUDE_DIRECTORIES ${fhiclcpp_INCLUDE_DIR}
          INTERFACE_LINK_LIBRARIES linedoc::Headers
      )
  endif()
endif()