# Agent Instructions

- This workspace is a Zephyr motor-control project (BLDC). The application is in `app/`, the MicroSpora board files are in `boards/st/microspora/`, and local modules are in `modules/lib/`.
- Preserve existing user changes; inspect a file before editing it and keep changes scoped to the request.
- Build with: `Build` task.
- Do not flash or run motor-control firmware unless the user explicitly asks. Tests can energize the motor; use current-limited, secured hardware for requested tests.
- Prefer the existing Zephyr and Spinner APIs and keep changes consistent with nearby code.

# Rules
- Do not use sed, grep, ls or other shell commands directly; use buildin tools.