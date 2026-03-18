HI_PYTHON ?= python3

inc_path := ${TOP_DIR}/ace/media/dss/vdp/include
out_path := $(shell pwd)/out/hdmi/lib64/stub
stub_path := $(shell pwd)/ace/media/dss/vdp/stub

mkdir_stub := $(shell mkdir -p $(out_path))
# The trimmed tree no longer carries the original HDMI MPI headers, so skip
# eager stub generation during makefile parse.
local_stub := $(shell true)
