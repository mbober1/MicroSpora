west init -l app
west update

west build -b microspora app -DBOARD_ROOT=/home/user/workspace/project/ -DOVERLAY_CONFIG=debug.conf