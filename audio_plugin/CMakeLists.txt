 
message(STATUS "Configuring CroPaC-Binaural...")

# These plug-ins rely on the saf_sofa_reader module being enabled:
if(SAF_ENABLE_SOFA_READER_MODULE)
    add_subdirectory(_CroPaC_binaural_) 
else()
    message(STATUS "  Note: disabling, as it relies on the saf_sofa_reader module.")
endif()