#!/bin/sh

. ./test-init.sh

if [ "$1" = 'actual' ]; then
  "$CALCURSE" --read-only -D "$DATA_DIR"/ -c "$DATA_DIR/apts-event-001" \
    -d02/23/2013
elif [ "$1" = 'expected' ]; then
  cat <<EOD
02/23/13:
 * Event
EOD
else
  ./run-test "$0"
fi
