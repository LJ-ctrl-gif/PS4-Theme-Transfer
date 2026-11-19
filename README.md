# PS4 Theme Transfer

A local-network file transfer companion for Android and a homebrew PS4 receiver. The first milestone focuses on moving user-selected theme assets between devices over the same Wi-Fi network.

## Project goals

- Android app: select pictures, audio, and other user-owned files; send them to a PS4 on the same local network.
- PS4 homebrew app: receive files and organize them into predictable folders.
- Reverse transfer: browse received assets on PS4 and send selected files back to Android.
- Keep file transfer separate from theme authoring, PKG creation, and theme installation. Those are separate milestones and are not claimed to work here.

## Planned storage layout

```
/data/PS4ThemeCreator/
  Received/
    Pictures/
    Music/
    SoundEffects/
    Other/
  Themes/
    Drafts/
    Exported/
  Settings/
```

## Planned components

- `android_app/`: Flutter Android companion.
- `ps4_receiver/`: PS4 homebrew receiver source and packaging files.
- `docs/TRANSFER_PROTOCOL.md`: local transfer protocol and safety limits.

## Current status

**Milestone 0 — repository and protocol foundation.** The app implementations and PS4 package are not yet complete. Build artifacts will only be described as available after CI produces them successfully.

## Requirements

- Android phone and PS4 connected to the same trusted local Wi-Fi network.
- A jailbroken PS4 capable of running homebrew.
- Only transfer files you own or are authorized to use. This project does not provide game packages, copyrighted theme assets, or license bypasses.

## Roadmap

1. Define and test the local transfer protocol.
2. Implement Android file selection, connection setup, and upload/download.
3. Implement PS4 receiver, safe file naming, and storage management.
4. Add reverse transfer and asset browsing.
5. Add CI builds and verify installable artifacts on supported hardware.
