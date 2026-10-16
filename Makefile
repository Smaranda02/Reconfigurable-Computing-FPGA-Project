# SPDX-License-Identifier: MIT
# Copyright (c) 2026 ARH Laboratory
# Author: Alexandru Ulmamei <alexandru.ulmamei@upb.ro>

.PHONY: all csim csynth export hls vivado view view-bd view-impl package deploy run clean

KERNEL ?= all
REMOTE_DIR ?= /home/xilinx/jupyter_notebooks/flow_control_project
PY_SCRIPT ?= test_merge_sort.py

all:
	$(MAKE) hls KERNEL=all
	$(MAKE) vivado
	$(MAKE) package

csim:
	./scripts/run_hls.sh csim $(KERNEL)

csynth:
	./scripts/run_hls.sh csynth $(KERNEL)

export:
	./scripts/run_hls.sh export $(KERNEL)

hls:
	./scripts/run_hls.sh all $(KERNEL)

vivado:
	./scripts/run_vivado.sh

view:
	./scripts/view_vivado.sh "$(VIEW)"

view-bd:
	./scripts/view_vivado.sh bd

view-impl:
	./scripts/view_vivado.sh impl

package:
	./scripts/package_overlay.sh

deploy:
	@if [ -z "$(TARGET)" ]; then \
		echo "Usage: make deploy TARGET=board-ip-or-dns [REMOTE_DIR=$(REMOTE_DIR)]"; \
		exit 1; \
	fi
	./scripts/deploy_to_pynq.sh "$(TARGET)" "$(REMOTE_DIR)"

run:
	@if [ -z "$(TARGET)" ]; then \
		echo "Usage: make run TARGET=board-ip-or-dns [REMOTE_DIR=$(REMOTE_DIR)] [PY_SCRIPT=$(PY_SCRIPT)] [OVERLAY_NAME=lab03_overlay]"; \
		exit 1; \
	fi
	OVERLAY_NAME="$(OVERLAY_NAME)" ./scripts/run_on_pynq.sh "$(TARGET)" "$(REMOTE_DIR)" "$(PY_SCRIPT)"

clean:
	rm -rf hls/build vivado/build overlay/*.bit overlay/*.hwh
