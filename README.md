west init -l app
west update

west build -b microspora app -DBOARD_ROOT=/home/ubuntu/workspace -DOVERLAY_CONFIG=debug.conf
