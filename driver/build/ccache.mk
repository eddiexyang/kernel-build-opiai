
ifneq ($(USE_CCACHE),)

export CCACHE := $(TOP)/build/prebuilts/misc/linux-x86/ccache/ccache

export CCACHE_LOGFILE := $(TOP)/vender/hisi/build/delivery/mini/log/ccache.log
 # The default check uses size and modification time, causing false misses
 # since the mtime depends when the repo was checked out
export CCACHE_COMPILERCHECK := content
 # See man page, optimizations to get more cache hits
 # Ignore include file modification time since it will depend on when
 # the repo was checked out
export CCACHE_SLOPPINESS := include_file_mtime,file_macro
 # Turn all preprocessor absolute paths into relative paths.
 # Fixes absolute paths in preprocessed source due to use of -g.
 # We don't really use system headers much so the rootdir is
 # fine; ensures these paths are relative for all Android trees
 # on a workstation.
export CCACHE_BASEDIR := /

endif

