#!/bin/sh

/usr/bin/rikcgi-fan --init

echo "Starting rikbtnd ..."
exec /usr/local/bin/rikbtnd

