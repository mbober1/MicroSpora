# SPDX-License-Identifier: Apache-2.0

# keep first
board_runner_args(jlink "--device=STM32G431CB" "--speed=4000")

# keep first
include(${ZEPHYR_BASE}/boards/common/jlink.board.cmake)
