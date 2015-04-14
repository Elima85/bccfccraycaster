
#    
# Processor sources
#
SOURCES += \
    $${VRN_MODULE_DIR}/bcc/bccvolumeraycaster.cpp \
	$${VRN_MODULE_DIR}/bcc/fccvolumeraycaster.cpp \
	$${VRN_MODULE_DIR}/bcc/unbiasedvolumeraycaster.cpp \
	$${VRN_MODULE_DIR}/bcc/volumeinterleave.cpp \

# 
# Processor headers
#
HEADERS += \
    $${VRN_MODULE_DIR}/bcc/bccvolumeraycaster.h \
    $${VRN_MODULE_DIR}/bcc/fccvolumeraycaster.h \
	$${VRN_MODULE_DIR}/bcc/unbiasedvolumeraycaster.h \
	$${VRN_MODULE_DIR}/bcc/volumeinterleave.h \

#
# Shader sources
#
SHADER_SOURCES += \
	$${VRN_MODULE_DIR}/bcc/glsl/rc_bccvolume.frag \
	$${VRN_MODULE_DIR}/bcc/glsl/rc_fccvolume.frag \
	$${VRN_MODULE_DIR}/bcc/glsl/rc_unbiased.frag

### Local Variables:
### mode:conf-unix
### End:
