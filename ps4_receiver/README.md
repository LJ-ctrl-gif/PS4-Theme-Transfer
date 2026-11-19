# PS4 Receiver

This directory contains the native receiver source for the PS4 homebrew app.

## Target

- PS4 Slim running compatible homebrew.
- OpenOrbis PS4 toolchain (not included).
- Local trusted Wi-Fi network shared with the Android device.

## Current implementation status

The first source milestone provides the receiver's local HTTP service and safe category-based file storage. It is source code only: no installable PKG is published until a reproducible OpenOrbis build and on-console test succeed.

## Build

The Makefile expects the OpenOrbis toolchain path in `OO_PS4_TOOLCHAIN`. Toolchain-specific packaging metadata and assets will be added after the compiler target is validated in CI.

Do not expose the receiver through router port forwarding. Use it only on a trusted local network.
