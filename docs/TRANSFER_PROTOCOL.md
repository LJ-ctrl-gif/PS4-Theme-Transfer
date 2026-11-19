# Local Transfer Protocol (v1 draft)

## Network model

Both devices must be on the same trusted Wi-Fi network. The initial implementation uses direct HTTP over the local network; it does not require a cloud account or public relay.

## Endpoints

A device acting as receiver exposes:

- `GET /api/v1/status` — app name, protocol version, and available categories.
- `POST /api/v1/upload?category=pictures` — upload one file as the request body.
- `GET /api/v1/files?category=pictures` — list files in a category.
- `GET /api/v1/download/<file-id>` — download a received file.

Both apps will implement this contract so either device can act as sender or receiver.

## Categories

- `pictures`
- `music`
- `sound-effects`
- `other`

## Safety and validation

- Bind to the local network only; never configure router port forwarding.
- Require the user to initiate pairing from the other device.
- Validate category names against the fixed allow-list.
- Treat incoming file names as untrusted. Strip path separators, control characters, and traversal components; generate unique destination names on collisions.
- Enforce a configurable maximum file size and reject empty uploads.
- Write to a temporary file first, then rename after upload completes.
- Never execute, install, or automatically open received files.
- Never accept arbitrary destination paths from a sender.

## Storage mapping

| Category | PS4 destination |
|---|---|
| pictures | `/data/PS4ThemeCreator/Received/Pictures/` |
| music | `/data/PS4ThemeCreator/Received/Music/` |
| sound-effects | `/data/PS4ThemeCreator/Received/SoundEffects/` |
| other | `/data/PS4ThemeCreator/Received/Other/` |

## Authentication note

The first local prototype may use a short pairing code shown on the receiver and entered by the sender. Do not use this protocol on public Wi-Fi. HTTP on a local network is not encrypted; HTTPS and stronger pairing are future hardening work.
