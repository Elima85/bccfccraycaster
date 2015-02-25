
#    
# Processor sources
#
SOURCES += \
    $${VRN_MODULE_DIR}/bcc/bccvolumeraycaster.cpp \
	$${VRN_MODULE_DIR}/bcc/fccvolumeraycaster.cpp \
	$${VRN_MODULE_DIR}/bcc/bccinterleavedvolumeraycaster.cpp \

# 
# Processor headers
#
HEADERS += \
    $${VRN_MODULE_DIR}/bcc/bccvolumeraycaster.h \
    $${VRN_MODULE_DIR}/bcc/fccvolumeraycaster.h \
	$${VRN_MODULE_DIR}/bcc/bccinerleavedvolumeraycaster.h \

#
# Shader sources
#
SHADER_SOURCES += \
	$${VRN_MODULE_DIR}/bcc/glsl/rc_bccvolume.glsl \
	$${VRN_MODULE_DIR}/bcc/glsl/rc_fccvolume.glsl

### Local Variables:
### mode:conf-unix
### End:
