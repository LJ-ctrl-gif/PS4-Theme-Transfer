# Implementation Status

## Implemented in source

- Public repository and project layout.
- Draft v1 HTTP endpoint contract.
- Android Flutter UI with manual receiver address, status check, category selection, and single-file upload flow.
- PS4 C++ HTTP listener source with a status endpoint and category-limited upload destination mapping.
- Android GitHub Actions workflow intended to generate the Android platform folder, analyze, and build a release APK.

## Not implemented yet

- Android file listing and download from PS4.
- PS4 file listing and download endpoints.
- PS4 on-screen UI, pairing code, and installable PKG packaging.
- Robust HTTP parsing, concurrent clients, progress reporting, duplicate-name handling, and stronger transport security.
- End-to-end testing on the target PS4 and Android phone.

## Important prototype limitations

The current PS4 receiver source is an early prototype, not a validated production HTTP server. It uses plain HTTP and must remain on a trusted local network. The upload handler currently accepts one request at a time and has a 32 MiB per-file limit. Do not expose it to the internet or use it for sensitive files.

No APK or PS4 PKG should be treated as released until a successful build artifact exists and the app has been tested on hardware.
