# Vox Avatar — PS5 Profile Avatar Injector 👤

A PS5 payload that injects a profile **avatar / profile picture** on your
console. Built with the
[ps5-payload-sdk (prospero)](https://github.com/ps5-payload-dev/ps5-payload-sdk).

- ✅ x86-64 PS5 payload (same format as `kstuff.elf` / `etaHEN-2.6.elf`)
- ✅ Reads staged avatar images from `/data/ps5upload/profile/`
- ✅ Installs them into `/system_data/priv/cache/profile/` per user account
- ✅ Shows a TV notification when done
- ✅ Works on jailbroken PS5 via the standard ELF loader (port `9021`)

## How it works

1. **Prepare the staging folder** on the console — each account gets its own
   sub-folder named after its user id in hex (`0x...`):

   ```
   /data/ps5upload/profile/
       0x0000000100000002/
           avatar.png
       0x0000000100000003/
           pic.jpg
   ```

2. **Run the payload** (see usage below). It will:

   - create `/system_data/priv/cache/profile/0x...`
   - copy every staged image into the matching user folder
   - restart/refresh the profile cache so the new avatar is picked up

3. A TV notification reports the result:

   ```
   vox.avatar OK: 2 user(s), 3 file(s)
   ```

## Usage

1. Jailbreak your PS5 (e.g. `etaHEN` / `pldmgr`).
2. Stage your avatars under `/data/ps5upload/profile/0x<user id>/`.
3. Send the payload:

   ```bash
   prospero-deploy -h <PS5_IP> -p 9021 vox.avatar.elf
   ```

Or just load `vox.avatar.elf` from your favourite loader (Vox Manager,
WebMAN, pldmgr toolbox, etc.).

## Build

Requirements:

- [ps5-payload-sdk](https://github.com/ps5-payload-dev/ps5-payload-sdk)
- LLVM / clang for x86_64 (used through the SDK toolchain)

```bash
make PS5_PAYLOAD_SDK=/path/to/ps5-payload-sdk
```

Output: `vox.avatar.elf`

## Notes

- User folders are detected by the `0x` hex prefix, matching how the system
  stores user ids under `/system_data/priv/cache/profile/`.
- Only regular files are copied; hidden files (`.` prefix) are skipped.